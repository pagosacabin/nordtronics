---
task_id: "0059"
protocol_version: 1.0.0
status: superseded  # superseded by 0064 (verified)
iteration: 1
expect-reply-within: 6h
notes: |
  BLOCKED — first blocking failure at runbook step 8. Deviation declared,
  awaiting a decision. This is a runbook gap, not a transient error; a re-run
  reproduces it exactly.

  Cause: backend/DEPLOY.md step 4 grants the *mosquitto* user ACL access to
  /etc/letsencrypt/live and /etc/letsencrypt/archive, but step 7 points the
  ingest worker at the same path
  (WILDFIRE_MQTT_CA=/etc/letsencrypt/live/nordtronics.io/chain.pem) and grants
  wildfire-ingest nothing. The directory is drwxr-x---+ root:root with
  `other::---` and a `user:mosquitto:r-x` ACL only, so an unprivileged
  wildfire-ingest cannot traverse into it.

  Exact failure (journalctl -u wildfire-ingest):
    File "/opt/nordtronics/backend/ingest/ingest/worker.py", line 96, in build_client
      if self.config.mqtt_ca_file and Path(self.config.mqtt_ca_file).exists():
      PermissionError: [Errno 13] Permission denied:
        '/etc/letsencrypt/live/nordtronics.io/chain.pem'
    wildfire-ingest.service: Main process exited, code=exited, status=1/FAILURE
    wildfire-ingest.service: Failed with result 'exit-code'.
  Reproduced per user, same host, same moment:
    sudo -u mosquitto  cat /etc/letsencrypt/live/nordtronics.io/chain.pem
      -> -----BEGIN CERTIFICATE-----
    sudo -u wildfire-ingest cat /etc/letsencrypt/live/nordtronics.io/chain.pem
      -> Permission denied
    sudo -u wildfire-ingest ls /etc/letsencrypt/live/nordtronics.io/
      -> Permission denied
  The worker's DB step succeeds before the crash ("database ready at
  /var/lib/nordtronics/wildfire.db (schema v1)"), so the CA read is the single
  blocker.

  Contributing code defect: pathlib Path.exists() swallows ENOENT/ENOTDIR but
  not EACCES, so an unreadable CA raises instead of being treated as absent.
  The worker hard-crashes rather than logging "CA unreadable".

  Verified working at the stop point:
   - mosquitto: active, FragmentPath=/etc/systemd/system/mosquitto.service,
     ExecStart=/usr/sbin/mosquitto -c /etc/mosquitto/nordtronics.conf; journal
     shows "Opening ipv4 listen socket on port 8883" and "Opening ipv6 listen
     socket on port 8883", "mosquitto version 2.0.18 running", and no cert/key
     error.
   - wildfire-api: active, listening 127.0.0.1:8000, User=wildfire-api.
   - wildfire-ingest: activating (Restart=on-failure loop).
  Steps 9 (nginx) and 10 (end-to-end verification) NOT run — the spec says stop
  at the first blocking failure.

  Declared deviations from the literal runbook, all made before the failure:
   a) Step 3 cloned --branch hermes/0056-backend-v1, not --branch main: main has
      no backend/ tree at all; the task says take the tree from the branch.
      Checked-out HEAD verified == 697ab6647ee3faba3114d111d4a51f93c8a29d9d,
      the SHA the task names.
   b) Step 3, added `chown -R root:root /opt/nordtronics/backend` to match the
      runbook's own path/owner table; the literal `rsync -a` preserved the
      deploy-owned clone (~/nordtronics-src belongs to deploy).
   c) Step 8, `systemctl stop mosquitto` immediately before `enable --now`: apt
      in step 1 had already started the packaged broker ("Created symlink
      .../multi-user.target.wants/mosquitto.service"), and `enable --now`
      against a running unit is a no-op — it would have left a broker with no
      8883 listener. Verified our unit is the live one via FragmentPath and
      ExecStart.
   d) Step 5, recorded node-01's password to
      /etc/nordtronics/mqtt-credentials.env (0600 root:root) because the runbook
      says "record it for the base station" without naming a location. No secret
      value appears in this file, in git, or in the reply.

  Observation, not a failure: mosquitto 2.0.18 warns that
  /etc/mosquitto/passwd and /etc/mosquitto/acl are owned by root rather than
  mosquitto ("Future versions will refuse to load this file"). Both load today
  at root:mosquitto 0640, exactly as the runbook specifies — but note the
  runbook's ownership is what a future mosquitto release will reject.

  Decision needed: how the ingest worker should trust the broker's CA. Either
  grant wildfire-ingest the same rX ACL step 4 gives mosquitto, or install a
  readable copy of chain.pem (e.g. under /etc/nordtronics/) and repoint
  WILDFIRE_MQTT_CA at it. Both change the runbook; neither is mine to choose
  under "do not improvise around it". No ACL or config change was made.
---

# 0059 — Deploy backend v1 to the VPS

## Context

SSH access is proven (0058: first-attempt key auth from your laptop as
`deploy`). The backend tree is on branch `hermes/0056-backend-v1`
(@ 697ab6647ee3faba3114d111d4a51f93c8a29d9d, CI green) and its deploy runbook
is `backend/DEPLOY.md` at that branch tip — 12 steps plus rollback and
troubleshooting, written for a second person. The VPS is 89.117.21.105,
Ubuntu 24.04, already hardened (UFW 22/80/443/8883, Let's Encrypt cert
covering nordtronics.io + api + mqtt).

## Task

SSH to the VPS as `deploy`, get the backend tree from the branch, and work
through `backend/DEPLOY.md` steps 1–10 in order, ending with the full
end-to-end verification in step 10:

1. All three services active: `systemctl is-active mosquitto wildfire-ingest wildfire-api`
2. Broker log shows the 8883 listen socket and no cert/key error
3. `curl http://127.0.0.1:8000/healthz` and `https://api.nordtronics.io/healthz` both answer
4. Publish one test reading as node-01 through the real TLS path and read it back from `/v1/nodes` and `/v1/nodes/node-01/readings`
5. Anonymous publish is refused; plaintext 1883 times out

## Success criteria

1. Every step-10 check passes, with the actual command outputs quoted in the reply.
2. Nothing runs as root or `deploy` — mosquitto, wildfire-ingest, wildfire-api each under its own service user.
3. The test reading round-trips: published via MQTT, visible in the API.

## Constraints

- Cost bound: deepseek-flash only, off-peak hours.
- MQTT passwords are generated on the VPS at deploy time and never committed anywhere.
- Follow the runbook. If a step fails, stop at the first blocking failure, report the exact output, and do not improvise around it — declare the deviation and wait.
- The node-01 test reading is synthetic and clearly labeled; delete it from the DB after verification so the database starts clean.

## Proof

The staged reply quoting the step-10 verification outputs (systemctl, journalctl lines, curl outputs, mosquitto_pub results including the two refusal cases).

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: each verification check with its output, and any deviations from
the runbook with reasons.

---

## Reply — step 8 pickup (2026-09-25): BLOCKED

Not staged: the task is not done, so there is no `proof` block and no success
claim. The file stays in `active/` with `status: in_progress`, and the blocking
failure plus every deviation is declared in the front-matter `notes` above —
per "stop at the first blocking failure ... declare the deviation and wait".

Steps 1–8 were executed in order on 89.117.21.105. Steps 1–7 completed and were
verified. Step 8 started all three units but `wildfire-ingest` could not read
the broker CA and entered a restart loop, so steps 9 and 10 were not run.

### Step-by-step state at the stop point

| Step | State |
|---|---|
| 1 packages | done — mosquitto 2.0.18, mosquitto-clients, acl, python3-venv, ca-certificates |
| 2 users | done — `wildfire-data:x:113:wildfire-ingest,wildfire-api`; ingest uid 110, api uid 111 |
| 3 tree + venv | done — HEAD `697ab664…29d9d`; venv /opt/nordtronics/venv (pip 26.2.1, fastapi 0.141.1, paho-mqtt 2.1.0, uvicorn 0.54.0) |
| 4 broker config/ACL/cert | done — nordtronics.conf + acl at root:mosquitto 0640; `sudo -u mosquitto cat …/privkey.pem` prints `-----BEGIN PRIVATE KEY-----` |
| 5 MQTT users | done — passwd holds `wildfire-ingest`, `node-01`; root:mosquitto 0640; node-01 credential recorded on the VPS only |
| 6 database dir | done — `drwxrws--- wildfire-ingest wildfire-data` |
| 7 ingest.env | done — root:root 0600, 7 keys, values not printed |
| 8 systemd units | **FAILED for wildfire-ingest** — see notes |
| 9 nginx | not run |
| 10 verification | not run |

### What step 8 verified

```
systemctl is-active mosquitto wildfire-ingest wildfire-api
active
activating
active

systemctl show mosquitto -p FragmentPath -p ExecStart
FragmentPath=/etc/systemd/system/mosquitto.service
ExecStart={ path=/usr/sbin/mosquitto ; argv[]=/usr/sbin/mosquitto -c /etc/mosquitto/nordtronics.conf ; ... }

sudo ss -lntp | grep -E '8883|8000'
LISTEN 0  100      0.0.0.0:8883  users:(("mosquitto",pid=15241,fd=4))
LISTEN 0  100         [::]:8883  users:(("mosquitto",pid=15241,fd=5))
LISTEN 0 2048  127.0.0.1:8000    users:(("python",pid=15263,fd=6))
```

Service users: `User=wildfire-ingest` and `User=wildfire-api` on their units;
mosquitto runs as the packaged `mosquitto` user. Nothing runs as root or
`deploy`, so success criterion 2 holds as far as it has been reached.

### What is not verified

Step 10's five checks. Checks 1 (all three active), 4 (TLS round-trip) and 5
(anonymous refused, 1883 times out) cannot pass while the ingest worker is down.
Checks 2 and 3 are partially evidenced — the broker does hold the 8883 socket
with no cert error and the API is listening — but neither the loopback nor the
nginx `https://api.nordtronics.io/healthz` response was requested, because step 9
was not run. No test reading was published, so no synthetic row exists to delete
and that constraint is untouched.
