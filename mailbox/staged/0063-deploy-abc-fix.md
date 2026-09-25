---
task_id: "0063"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  branch: hermes/0063-deploy-abc-fix
  sha: 95de2ae6bdd7b60906a6583b941175b603694715
  run: https://github.com/pagosacabin/nordtronics/actions/runs/36170346279
  files:
    - backend/DEPLOY.md
    - backend/mosquitto/renew-hook.sh
    - backend/ingest/wildfire-ingest.service
    - backend/api/wildfire-api.service
notes: |
  All three fixes implemented on hermes/0063-deploy-abc-fix @ 95de2ae6bdd7;
  CI green at that tip (run 36170346279, both jobs, artifacts read). Branch
  only: nothing on 89.117.21.105 was written, chmod'ed or restarted — only
  read-only probes, declared in the reply.

  Two corrections worth flagging before the apply task:
  (1) fix C's mechanism is narrower than 0062 concluded. A umask really cannot
      create the group-write bit, but SQLite gives a new -shm/-wal the mode of
      the database FILE it belongs to, so 0660 on wildfire.db is what makes the
      sidecars shareable — measured with the repo's own init_db (db 0660 ->
      sidecars 0660; db absent -> 0640, today's lockout). The units' UMask=0007
      was already present at the base commit and is not load-bearing, so the
      unit diff is comment-only, correcting two comments that claimed a umask
      can grant group write.
  (2) the pinned bundle is 0640 root:wildfire-data and `deploy` is not in
      wildfire-data, so step 10's operator-side mosquitto_pub verifies against
      the system store (same X1 anchor) instead of the bundle. A read grant or
      0644 would let it use the bundle; left as a decision.
  Also declared: 0061's ingest setfacl pair in step 4 is kept (now redundant,
  comment corrected); mosquitto's own cafile is untouched; the preseeded
  database makes /healthz answer 500 instead of 503 until the worker writes
  the schema; the renew hook also restarts wildfire-ingest.
---

# 0063 — Fix the three deploy defects blocking 0059/0062 (branch only)

## Context

0062's resume run decision-blocked on three defects, all independently
verified by Juno over SSH on 2026-09-25 ~12:00 MDT. 0059 and 0062 stay
blocked until this lands. Branch from the 0061 tip:
`hermes/0061-ingest-ca-acl @ 9eba1cec9716f22779e6014190b6ef93b1082ad2`.
Name the new branch `hermes/0063-deploy-abc-fix`.

Verified facts (do not re-diagnose; implement the fixes):

- A — trust anchor: `/etc/letsencrypt/live/nordtronics.io/chain.pem` holds
  3 certs (YE1 -> Root YE -> ISRG Root X2, the last cross-signed by ISRG
  Root X1). The self-signed ISRG Root X1 is absent, so a TLS client using
  chain.pem as its trust store fails with `Verify return code: 2 (unable to
  get issuer certificate)` — reproduced with openssl s_client. The 0061 ACL
  fix gave wildfire-ingest read access, but read access to an unanchored
  bundle is not enough.
- B — hostname: the served cert's SANs are DNS-only (`api`, `mqtt`,
  apex, `www`). `WILDFIRE_MQTT_HOST=127.0.0.1` can never pass hostname
  verification.
- C — WAL ownership: `/var/lib/nordtronics/` is `wildfire-ingest`:
  `wildfire-data` mode 2770, but `wildfire.db-shm`/`-wal` are
  `wildfire-api`-owned mode 0640 — group lacks write, so wildfire-ingest
  dies with "attempt to write a readonly database" (840 restarts observed).

## Task

On the new branch, make these runbook (+ unit file) changes. No VPS
changes in this task.

1. `backend/DEPLOY.md`: add a step that builds a pinned CA bundle
   `/etc/nordtronics/mqtt-ca.pem` as
   `chain.pem` + the self-signed ISRG Root X1
   (`/usr/share/ca-certificates/mozilla/ISRG_Root_X1.crt`), owned
   `root:wildfire-data`, mode 0640; set `WILDFIRE_MQTT_CA` to that path;
   extend the cert renew-hook so a renewal rebuilds the bundle (chain
   changes must not silently unanchor it). Explain in one comment why
   chain.pem alone is insufficient (cross-signed root, absent X1 anchor).
2. `backend/DEPLOY.md`: set `WILDFIRE_MQTT_HOST=mqtt.nordtronics.io` and
   add `127.0.0.1 mqtt.nordtronics.io` to `/etc/hosts`, with a comment
   that the cert is DNS-only so the loopback name must verify.
3. `backend/DEPLOY.md` + the repo's systemd units (`wildfire-ingest` and
   `wildfire-api`): database files must be group `wildfire-data` mode
   0660 (directory 2770, already correct), and both units get
   `UMask=0007` so future `-shm`/`-wal` files are created group-writable.
   Readers need -shm write access in WAL mode — that is why 0660, not
   0640.

Push the branch and get CI green.

## Success criteria

1. CI is green on `hermes/0063-deploy-abc-fix`.
2. The diff implements exactly the three fixes above (plus tests if any
   behavior changed).

## Constraints

- Branch only: zero VPS changes. A follow-up task applies this on the VPS.
- Cost bound: deepseek-flash only, off-peak hours.
- Do not touch the active files for 0059 or 0062.

## Proof

Branch SHA on origin + Actions run URL showing green, both quoted in the
reply.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: the branch SHA, the CI run URL and result, and the diff stat.

---

## Reply — three fixes implemented, CI green at the tip (2026-09-25)

Branch `hermes/0063-deploy-abc-fix`, tip `95de2ae6bdd7b60906a6583b941175b603694715`,
branched from `hermes/0061-ingest-ca-acl @ 9eba1cec9716f22779e6014190b6ef93b1082ad2`
(`git rev-parse hermes/0063-deploy-abc-fix^` = `9eba1cec…`).

```
$ git ls-remote --heads origin hermes/0063-deploy-abc-fix
95de2ae6bdd7b60906a6583b941175b603694715	refs/heads/hermes/0063-deploy-abc-fix

$ gh run view 36170346279 --json headSha,conclusion,status
{"conclusion":"success","headSha":"95de2ae6bdd7b60906a6583b941175b603694715",
 "status":"completed","url":"https://github.com/pagosacabin/nordtronics/actions/runs/36170346279"}
```

`headSha` equals the branch tip, so the run is not a stale pointer.

**CI: https://github.com/pagosacabin/nordtronics/actions/runs/36170346279 —
success**, 2026-09-25T17:57Z, both jobs:

| Job | Result | Evidence I read |
|---|---|---|
| Ingest + API tests | success | downloaded the `backend-test-report` artifact: `121 passed, 1 warning in 1.04s` |
| Mosquitto config check | success | downloaded the `mosquitto-config-check` artifact: ends `mosquitto config: ALL CHECKS PASSED` (TLS 8883 up, anonymous publish/subscribe refused, ingest ACL read works, node-01 cannot publish to node-02's topic) |

That workflow also runs `shellcheck backend/mosquitto/*.sh` and
`systemd-analyze verify` over all three units, so the changed hook and the
edited units are lint-verified in CI. I reproduced both steps locally first
(shellcheck 0.11.0: clean; `systemd-analyze verify`: clean) plus `sh -n` over
all 19 bash blocks in `DEPLOY.md` (0 syntax errors).

Diff, `origin/hermes/0061-ingest-ca-acl..hermes/0063-deploy-abc-fix`
(4 files, +143 −26):

| File | +/− |
|---|---|
| `backend/DEPLOY.md` | 93 / 18 |
| `backend/mosquitto/renew-hook.sh` | 38 / 3 |
| `backend/ingest/wildfire-ingest.service` | 7 / 3 |
| `backend/api/wildfire-api.service` | 5 / 2 |

### Fix A — the trust anchor, verified against the live broker

Step 7 builds `/etc/nordtronics/mqtt-ca.pem` as `chain.pem` +
`/usr/share/ca-certificates/mozilla/ISRG_Root_X1.crt`, owned
`root:wildfire-data` 0640, and points `WILDFIRE_MQTT_CA` at it. The step comment
states why `chain.pem` alone cannot work, and step 4's install of the deploy
hook now says the hook has two jobs.

I verified the bundle instead of assuming it works. Read-only on the VPS:
`chain.pem` holds 3 certs — leaf `CN=YE1` → `CN=Root YE` → `CN=ISRG Root X2`
with `issuer=CN=ISRG Root X1` — and the X1 file is self-signed
(`subject` == `issuer`). Then, from here, against the live broker:

```
CAfile = bundle (chain.pem + self-signed ISRG Root X1) -> Verify return code: 0 (ok)
CAfile = chain.pem (the value deployed today)          -> Verify return code: 2 (unable to get issuer certificate)
```

So this exact bundle anchors the served chain, and the current value fails
exactly as 0062 reported.

The renew hook now rebuilds the bundle on every renewal (same two sources,
`chown root:wildfire-data`, `chmod 0640`), **refuses** to write a bundle when
the X1 anchor is missing (a hook that silently produced an unanchored CA would
take the worker down long after the renewal that caused it), reloads mosquitto,
and restarts `wildfire-ingest` — the worker holds the CA it read at startup, so
without that restart a renewal would leave it trusting the old chain.

### Fix B — the hostname

Step 7's `ingest.env` now carries `WILDFIRE_MQTT_HOST=mqtt.nordtronics.io`, and
the step adds the `127.0.0.1 mqtt.nordtronics.io` line to `/etc/hosts`
(idempotently: appended only if that exact line is absent), so the worker
verifies the name that is in the certificate while the traffic stays on
loopback. The paragraph that invited `127.0.0.1` ("so the certificate name does
not have to resolve here") is replaced by the correct explanation, including
that both halves are needed: an anchored bundle does not help while the host is
an IP, and the right name does not help while the CA cannot build a path.

### Fix C — the WAL files, and a correction to 0062's mechanism

Step 6 now creates the database file at 0660 and fixes any existing sidecars:

```bash
sudo -u wildfire-ingest touch /var/lib/nordtronics/wildfire.db
sudo find /var/lib/nordtronics -maxdepth 1 -name 'wildfire.db*' \
  -exec chown wildfire-ingest:wildfire-data {} + -exec chmod 0660 {} +
```

`touch` never truncates, so this is safe to run against the live database with
its data in it — which is what the apply task will do.

The mechanism is narrower than 0062 concluded. 0062 wrote that "a umask change
cannot work: the base mode has no group-write bit to preserve" — true only for
a database SQLite has to *create*. SQLite gives a new `-shm`/`-wal` sidecar the
mode of the database file it belongs to, so 0660 on `wildfire.db` is what makes
the sidecars shareable, whoever creates them. Measured with the repo's own
`common.db.init_db`, umask 0007 in every case, modes stat'ed while the
connection was open:

| pre-existing `wildfire.db` | db / `-shm` / `-wal` |
|---|---|
| none (SQLite creates it) | 0640 / 0640 / 0640 ← today's lockout |
| touched, left 0644 | 0644 / 0644 / 0644 |
| touched, chmod 0660 | **0660 / 0660 / 0660** |

Every run also reported `schema_version=1`, i.e. a zero-length file that has
been `touch`ed is a valid starting point for the worker's `init_db`.

Declared consequence: the units already had `UMask=0007` at the base commit, so
the unit diff is **comment-only** — it corrects the two comments that claimed a
umask can make the sidecars group-writable. If you would rather the units
enforce the mode themselves (an `ExecStartPre=+/usr/bin/chmod 0660 …` that
re-heals if someone deletes and recreates the database), that is a small
addition; I left it out because the spec asked for the mode on the files and the
runbook now creates them.

### Also declared — choices the spec did not pin down

1. **`deploy` cannot read the pinned bundle.** It is `0640 root:wildfire-data`
   and step 2 does not put `deploy` in `wildfire-data` (checked on the VPS:
   `groups=sudo,users`), so step 10's operator-side `mosquitto_pub` verifies
   against `/etc/ssl/certs/ca-certificates.crt`, which holds the same
   self-signed X1 and anchors the identical chain (0062's probe showed that
   store works). If you want that check to use the pinned bundle, it needs a
   read grant (one `setfacl`) or 0644 — your call, not mine to widen silently.
2. **Step 10's plaintext-1883 check now addresses `89.117.21.105`.** With
   `mqtt.nordtronics.io` pinned to loopback, that check would be satisfied by a
   refused connection to a listener that does not exist rather than a firewall
   drop; the public IP keeps it testing UFW. (The broker config has no 1883
   listener at all.)
3. **0061's ingest `setfacl` pair stays.** After fix A the worker no longer
   reads `chain.pem`, so the grant is no longer load-bearing; I corrected the
   comment that claimed otherwise and left the lines in place rather than
   deleting a previous task's fix on my own authority. Removing them is a
   two-line change if you want it.
4. **`mosquitto`'s own `cafile …/chain.pem` is untouched.** It is only used for
   client certificates, `require_certificate` is unset, so nothing verifies
   against it today (0062 flagged this); repointing it would also mean creating
   a bundle fixture for CI's broker check.
5. **The preseeded database changes one error path.** Until the worker applies
   the schema, `/healthz` answers `500 internal error` (`no such table: meta`,
   reproduced locally against a zero-length file) instead of the `503 telemetry
   database is not available` it returns for a missing file. Step 6 and the
   troubleshooting table both say so; it clears as soon as step 8's worker has
   started once.
6. **The replacement error strings** (readonly database, unable to get issuer,
   IP address mismatch) are now in the troubleshooting table with the file mode
   / CA path / host that fixes each.

### Scope check

Branch only. On 89.117.21.105 nothing was written, chmod'ed, restarted or
otherwise changed: the probes were read-only (`id`, `ls`, `cat` of public
certificate material) and the certificate chain I used to test the bundle
locally was written only under this worker's scratch directory. The VPS tree is
still `hermes/0061-ingest-ca-acl @ 9eba1cec9716f2`, `ingest.env` still carries
`WILDFIRE_MQTT_HOST=127.0.0.1` and `WILDFIRE_MQTT_CA=…/chain.pem`, and
`wildfire-ingest` is still crash-looping. 0059 and 0062 were not touched, no
Python behaviour changed (hence no test changes — the suite is unchanged and
passes), and nothing was written to `handoff/outbox/`.
