---
task_id: "0132"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - branch: none
    note: "host-only apply task -- no branch was created and nothing was pushed except
      the mailbox state transitions on main. The deliverable is a file on the VPS
      (/etc/nordtronics/api.env) plus a service restart, neither of which can live in
      a git branch."
  - run: none
    note: "no CI run covers this deliverable and none can: no repository file changed,
      so no workflow could have been triggered by the task itself. Enumerated,
      not asserted: .github/workflows/ holds android-build.yml, android-companion-v0.yml,
      backend.yml, detection-sim.yml, platformio.yml, website-check.yml -- the build
      workflows fire on their own branch allow-lists (android-companion-v0.yml, etc.)
      and path filters, and none of their paths changed. The only runs on main for
      this run's commits are 'Website Check' on the mailbox-transition commits
      (d0e0f8c = success), which check the website, not this task."
  - applied_to_vps:
      - path: /etc/nordtronics/api.env
        owner: "root:root"
        mode: "0600"
        bytes: 61
        lines: 1
        content_shape: "exactly one line, `WILDFIRE_API_KEY=<key>` (17-byte name+`=` + 43-byte value + 1 newline = 61 B)"
        value_check: "value sha256 first 12 hex a4e1fb370c74, length 43 -- byte-equal
          to the source /home/astroboy/.hermes/api.key (43 B) and byte-equal to the
          value in the restarted service's own environment (/proc/<MainPID>/environ).
          The value itself was never printed anywhere."
      - path: "wildfire-api.service"
        action: "restarted (systemctl restart wildfire-api)"
        state: "active (running); NRestarts=0; EnvironmentFiles=/etc/nordtronics/api.env (ignore_errors=yes)"
  - evidence: "/etc/nordtronics/api.env: ls -la (owner/mode only), wc -c -l, an
      awk line count, and per-name value sha256 -- no `cat` of the file was ever run.
      The service-side check is /proc/<MainPID>/environ filtered to the variable NAME
      plus its value hash. Transcript lines are quoted in the reply body; pasted
      output is a claim, the host is the artefact."
  - files: []
    reason: "no repository file changed -- key placement on the VPS and a service
      restart only; the only commits this run made to main are the mailbox transitions
      (pickup d0e0f8c, this staging)."
notes: |
  STAGED (iteration 1), 2026-10-08 21:15-21:20 UTC / 23:15-23:20 CEST (VPS clock).
  Off-peak (PEAK: OFF-PEAK 21:15 UTC). PROTOCOL: MATCH protocol_version=1.0.0
  (origin/main:mailbox/README.md). Model tier: DeepSeek Flash (deepseek-flash /
  provider deepseek) -- the cron worker's cheap tier.

  WHAT CHANGED. One host, one file, one restart, nothing else:
  (1) created /etc/nordtronics/api.env on the VPS with exactly one line,
      `WILDFIRE_API_KEY=<43-byte key>`, owner root:root, mode 0600 -- the same
      convention as the sibling /etc/nordtronics/ingest.env (also root:root 0600).
  (2) `sudo systemctl restart wildfire-api`.
  No backend code was deployed or touched; /opt/nordtronics/backend is exactly as it
  was (still the rsync'd pre-auth tree -- /v1/nodes still answers 200 without a key,
  which the task explicitly says to expect and NOT to "fix").

  KEY DELIVERY PATH (laptop file -> SSH -> /etc/nordtronics/api.env, per Constraints).
  The key travelled as stdin only: `{ printf 'WILDFIRE_API_KEY='; tr -d '\n' <
  ~/.hermes/api.key; printf '\n'; } | ssh deploy@89.117.21.105 "sudo sh -c 'umask 077;
  cat > /etc/nordtronics/api.env'"`. It was never an argument to any local or remote
  process (so it never appears in a process table or shell history), never written to
  a local file, never echoed, cat'd or logged. `umask 077` creates the file at 0600;
  owner (root, from sudo) and mode were then set explicitly.

  VERIFICATION PERFORMED (and read back from the host, not assumed):
  - `ls -la /etc/nordtronics/api.env` -> `-rw------- 1 root root 61 Oct  8 23:16`.
  - `wc -c -l` -> 1 line, 61 bytes; `awk 'END{print NR}'` -> 1 (single line, no extras).
  - `grep -o '^WILDFIRE_API_KEY='` -> the variable name matches once.
  - Value integrity without disclosure: the value is 43 bytes and hashes to
    a4e1fb370c74 (sha256, first 12 hex) -- identical to the source key file and to
    the value visible in the restarted process's environment. That is the same key
    hash 0130 recorded for the APK build (a4e1fb370c74), so laptop, service env and
    the shipped APK are provably the same key.
  - `systemctl is-active wildfire-api` -> active; status shows `active (running)`
    since 2026-10-08 23:16:18 CEST, NRestarts=0.
  - `systemctl show -p EnvironmentFiles` -> `/etc/nordtronics/api.env
    (ignore_errors=yes)` -- the unit's `EnvironmentFile=-` reference, now resolving.
  - `curl -s -o /dev/null -w '%{http_code}' http://127.0.0.1:8000/healthz` -> 200 with
    NO key, after the restart (the criterion).
  - End-to-end delivery: `sudo cat /proc/159683/environ | tr '\0' '\n' | grep -o
    '^WILDFIRE_API_KEY'` -> present, value 43 bytes / sha a4e1fb370c74. So the file is
    actually consumed by the service, not merely present on disk.
  Note what was NOT proved: the variable was not sampled in the process environment
  before the restart, so "absent before" rests on the file not existing plus the unit's
  `EnvironmentFile=-` (ignore_errors), not on a pre-change measurement.

  HYGIENE. A byte scan of the whole repo worktree (excluding .git and the gitignored
  .gradle/build trees) and of all 467 tracked blobs for the 43-byte key found ZERO
  hits. The key is not in the repo, not in this reply, not in any log, not in chat, and
  not in any file on the worker host other than the one Stephen placed
  (/home/astroboy/.hermes/api.key, mode 0644, left as-is). The only copy on the VPS is
  /etc/nordtronics/api.env. Nothing in this run prints it.

  DEVIATIONS: (1) nothing outside key placement + restart + verify was touched on the
  VPS; (2) no branch and no CI run exist for this task -- this is an apply task, and
  the enumeration of why no run can exist is in `proof.run`; (3) the pre-change process
  environment was not sampled (see note above). No ntfy receipt: the spec names no
  topic and this produces no compiled artifact -- the ntfy convention in mailbox/README
  is for build tasks only.

  FOR JUNO: the api.env side is done, so your backend flip has the key waiting. After
  you deploy the auth code, /v1/* should start returning 401 without the header; if it
  still returns 200, the deployed code is still the pre-auth revision rather than a
  key problem.
---

# 0132 — Place API key in wildfire-api service environment on VPS

## Context

- The backend now gates every /v1/* route on the X-API-Key header (commit 9dfc091, on main). The API reads the server key from env var WILDFIRE_API_KEY via `EnvironmentFile=-/etc/nordtronics/api.env` in the wildfire-api.service unit.
- Stephen placed the shared API key at /home/astroboy/.hermes/api.key on your machine. The companion-app release APK (0130) already sends it.
- Current VPS state (checked 2026-10-08 by Juno): /etc/nordtronics/api.env does NOT exist; wildfire-api is active on the old (pre-auth) code; /opt/nordtronics/backend is an rsync'd tree, not a git checkout.

## Task

1. Read the key from /home/astroboy/.hermes/api.key.
2. As deploy on the VPS (89.117.21.105, passwordless sudo), create /etc/nordtronics/api.env containing exactly one line:
   WILDFIRE_API_KEY=<the key>
   Owner root:root, mode 0600 (matches /etc/nordtronics/ingest.env).
3. Restart the API: sudo systemctl restart wildfire-api.
4. Verify the service is active and /healthz returns 200 with NO key:
   curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8000/healthz
   NOTE: the pre-auth code is still deployed, so /v1/nodes will still return 200 without a key right now — that is expected. Juno deploys the auth code next and re-verifies the 401/200 gate herself. Do not "fix" this.

## Success criteria

- /etc/nordtronics/api.env exists, root:root, mode 0600, single WILDFIRE_API_KEY= line.
- wildfire-api active after restart; /healthz returns 200 with no key.

## Constraints

- The key NEVER goes in a git repo, a staged reply, a log, or chat. It travels only: your laptop file -> SSH -> /etc/nordtronics/api.env.
- Do not deploy backend code or change anything else on the VPS. Key placement + restart + verify only.
- Do not touch mosquitto config, ACLs, the database, or any other service.

## Proof

- SSH transcript showing: the api.env `ls -la` line (mode and owner only, NOT its contents), `systemctl is-active wildfire-api` output, and the /healthz HTTP code.
- State explicitly that the key value was not printed, logged, or committed anywhere.

## Reply format

Staged reply per mailbox/README.md: status, the proof pointers above, notes.


---

# Reply — Hermes → Juno — **STAGED**

**Model note:** cron worker on `deepseek-flash` (`deepseek`), the cheapest tier.

Outcome: **done**. `/etc/nordtronics/api.env` now exists on the VPS with the shared
key, and `wildfire-api` was restarted and is healthy. No repository file changed —
this is a host-only apply task.

## 1. SSH transcript (owner/mode and codes only — never the file's contents)

```
$ ssh deploy@89.117.21.105 'sudo ls -la /etc/nordtronics/api.env'
-rw-------    1 root     root            61 Oct  8 23:16 /etc/nordtronics/api.env

$ ssh deploy@89.117.21.105 'sudo wc -c -l /etc/nordtronics/api.env; sudo awk "END{print NR}" /etc/nordtronics/api.env'
 1 61 /etc/nordtronics/api.env
1

$ ssh deploy@89.117.21.105 'sudo grep -o "^WILDFIRE_API_KEY=" /etc/nordtronics/api.env'
WILDFIRE_API_KEY=

$ ssh deploy@89.117.21.105 'sudo ls -la /etc/nordtronics/ingest.env'   # convention reference
-rw-------    1 root     root           278 Sep 25 21:17 /etc/nordtronics/ingest.env

$ ssh deploy@89.117.21.105 'sudo systemctl restart wildfire-api; sleep 3; systemctl is-active wildfire-api'
active

● wildfire-api.service - Nordtronics wildfire HTTPS API
     Loaded: loaded (/etc/systemd/system/wildfire-api.service; enabled; preset: enabled)
     Active: active (running) since Thu 2026-10-08 23:16:18 CEST; 3s ago
   Main PID: 159683 (python)
      Tasks: 1 (limit: 9483)
     Memory: 34.7M (peak: 34.8M)
        CPU: 866ms
     CGroup: /system.slice/wildfire-api.service
             └─159683 /opt/nordtronics/venv/bin/python -m api.app

$ ssh deploy@89.117.21.105 'curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8000/healthz'
200                        # with NO key — the criterion

$ ssh deploy@89.117.21.105 'curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1:8000/v1/nodes'
200                        # EXPECTED: the pre-auth code is still deployed; Juno flips it.
                           # Not "fixed", per the task's NOTE.

$ ssh deploy@89.117.21.105 'systemctl show wildfire-api -p EnvironmentFiles -p NRestarts -p ActiveEnterTimestamp'
EnvironmentFiles=/etc/nordtronics/api.env (ignore_errors=yes)
NRestarts=0
ActiveEnterTimestamp=Thu 2026-10-08 23:16:18 CEST
```

## 2. Value integrity — proved without printing the value

| Check | Result |
|---|---|
| `/etc/nordtronics/api.env` size / line count | 61 B / **1 line** (17-byte `WILDFIRE_API_KEY=` + 43-byte value + newline) |
| owner : group / mode | `root:root` / `0600` (`-rw-------`) |
| value length | **43 bytes** |
| value sha256 (first 12 hex) | **a4e1fb370c74** |
| source `/home/astroboy/.hermes/api.key` length / first-12 hash | 43 B / **a4e1fb370c74** (identical) |
| value as seen by the running service (`/proc/159683/environ`) | present, 43 B, **a4e1fb370c74** (identical) |

The `a4e1fb370c74` hash is the same one 0130 recorded when the release APK was built,
so laptop file, VPS service environment and the shipped APK are provably the same key.

End-to-end delivery is therefore verified at both ends: the file on disk and the
variable inside the restarted process. Before the change the file did not exist (Juno's
own pre-check, re-confirmed here: `/etc/nordtronics/` held only `ingest.env`,
`mqtt-ca.pem`, `mqtt-credentials.env`), and the unit references it with
`EnvironmentFile=-` (ignore_errors), so the variable was necessarily absent pre-restart.
I did not sample the pre-restart process environment, so that half is inference, not
measurement, and is declared as such.

## 3. The key was not printed, logged, or committed anywhere

- Delivery over SSH used **stdin only** — the key was never an argument to any local or
  remote process, so it never entered a process table, an argv dump, or a shell history:
  `{ printf 'WILDFIRE_API_KEY='; tr -d '\n' < ~/.hermes/api.key; printf '\n'; } | ssh
  deploy@89.117.21.105 "sudo sh -c 'umask 077; cat > /etc/nordtronics/api.env'"`.
- It was written to **no local file**: the only local artifact in the pipeline is the
  pre-existing key file Stephen placed. Nothing was `cat`'d, `echo`'d, or logged.
- **Repo scan for the literal:** all 467 tracked blobs and every file in the worktree
  (excluding `.git` and the gitignored `.gradle`/`build` trees) scanned byte-wise for the
  43-byte key — **0 hits**.
- It is not in this reply, not in any ntfy message, and not in any chat message.
- The only copy on the VPS is `/etc/nordtronics/api.env` (0600 root:root).

## 4. Scope

Touched on the VPS: `/etc/nordtronics/api.env` (created), `wildfire-api` (restarted).
No backend code deployed or modified (`/opt/nordtronics/backend` is the same rsync'd
tree — `/v1/nodes` still answers 200 without a key on the pre-auth code, as expected).
Mosquitto config, ACLs, the database and every other service were left alone.

## 5. Handoff

Your flip has the key waiting. After you deploy the auth code, `/v1/*` should return
401 without `X-API-Key` and 200 with it; a lingering 200 means the deployed revision is
still pre-auth, not that the key is wrong.
