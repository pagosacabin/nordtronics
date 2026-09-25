---
task_id: "0063"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
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
