---
task_id: "0066"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0065-deploy-followups
    sha: a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36185130873
  - files: []
  - applied_to_vps:
      - "/etc/mosquitto/passwd -> mosquitto:mosquitto 0600 (was root:mosquitto 0640)"
      - "/etc/mosquitto/acl -> mosquitto:mosquitto 0640 (was root:mosquitto 0640)"
      - "/etc/nginx/sites-available/nordtronics.io -> api vhost gained the two snippet includes (backup nordtronics.io.bak-0066)"
      - "/etc/nordtronics/mqtt-ca.pem -> + ACL user:deploy:r-- (0640 root:wildfire-data unchanged)"
  - services:
      mosquitto: active
      wildfire-api: active
      wildfire-ingest: active
notes: |
  APPLIED on the VPS (89.117.21.105) as `deploy`: all three items, in runbook
  order, each with a before/after capture. The revision applied is the 0065
  runbook, hermes/0065-deploy-followups @ a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f,
  re-verified this run together with its CI run (headSha == live branch tip).

  No repository file changed (this is the apply task): proof.files is empty on
  purpose, and branch/sha/run name the runbook revision that was executed and the
  CI run that green-verified that exact revision.

  Item 1 (mosquitto passwd/acl ownership): both files moved to
  mosquitto:mosquitto, 0600 passwd / 0640 acl, then reloaded. The check is causal,
  not "no warning appeared in some window": a deliberate reload BEFORE the chown
  emitted both warnings (23:16:54), proving the reload path re-reads both files;
  the two config loads after the chown (23:16:57, 23:17:54) logged "Reloading
  config." with zero ownership warnings, and the count since 23:16:56 is 0.

  Item 2 (api vhost hardening): the two include lines were added to the api
  server block only, at the same position the apex block uses (after
  ssl_certificate_key). The diff against the pre-edit backup is exactly those two
  lines. nginx -t successful, reloaded, and https://api.nordtronics.io/v1/nodes
  serves 200 with HSTS / X-Content-Type-Options / X-Frame-Options /
  Referrer-Policy / Permissions-Policy / CSP now present - the same header set the
  apex serves.

  Item 3 (operator bundle access): setfacl -m u:deploy:r on the bundle. getfacl
  shows user:deploy:r--, the file stays -rw-r----- root:wildfire-data, and both
  readers were re-checked in the same pass (deploy exit 0, wildfire-ingest exit 0)
  so the ACL cannot have cost the worker its access.

  Declared corrections / deviations:
  1. The VPS deployed tree was NOT moved. 0065 changed only backend/DEPLOY.md,
     which is documentation and not part of the deployed tree, so no checkout or
     rsync was needed (unlike 0064). ~/nordtronics-src HEAD is still
     95de2ae6bdd7b60906a6583b941175b603694715 and /opt/nordtronics/backend was not
     written to. No source was edited on the VPS. The revision in proof is the
     runbook revision executed, not a tree that was deployed.
  2. `nginx -t` now emits the "ssl_stapling ignored, no OCSP responder URL"
     warning twice rather than once: the api vhost now includes
     nordtronics-tls.conf, which sets ssl_stapling on, and this certificate has no
     OCSP responder URL (openssl x509 -ocsp_uri is empty). Expected consequence of
     item 2 - a warning, not an error, the config test is successful, and it is the
     same pre-existing pattern the apex block already had. The tls-snippet include
     count in the site file went 1 -> 2.
  3. The two ownership warnings still inside a naive "last 10 minutes" journal
     window are the deliberate pre-fix probe at 23:16:54, not a failed fix; the
     count since 23:16:56 is 0.
  4. Actions beyond the literal three items: that one pre-fix reload, plus a second
     post-fix reload used as a clean re-check. Both were reloads, never restarts -
     the broker kept PID 15241 throughout, so no client was dropped.
  5. Scratch files this run created on the host (/tmp/0066-nginx-edit.py,
     /tmp/nginx-T-0066.txt) were removed at the end. The only persistent host
     artifacts are the four paths in proof.applied_to_vps plus the pre-edit backup
     /etc/nginx/sites-available/nordtronics.io.bak-0066.

  No secrets in this reply - no credential value was read or printed this run.
  Nothing written to handoff/outbox/. 0067 is still in mailbox/inbox/ and was not
  touched: this run worked one task, 0066.
---

# 0066 — Apply the 0065 follow-ups on the VPS

## Context

0065 is verified green (`hermes/0065-deploy-followups @
a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f`, CI run 36185130873 success).
Its runbook changes are the four deploy follow-ups from 0064's notes.
This task applies them on the VPS. Independent of 0067 (CI fixture),
which may run in any order.

## Task

SSH to the VPS as `deploy` and apply the 0065 runbook changes exactly:

1. Mosquitto file ownership: `chown mosquitto:mosquitto`
   `/etc/mosquitto/passwd` (mode 0600) and `/etc/mosquitto/acl` (mode
   0640); reload mosquitto; confirm the ownership warning is gone from
   the journal.
2. API vhost hardening: add the tls/headers snippet include lines to the
   api vhost in `/etc/nginx/sites-available/nordtronics.io` so it
   matches the site's hardening; `nginx -t`; reload nginx; verify
   `https://api.nordtronics.io/v1/nodes` still serves 200.
3. Operator bundle access: `setfacl -m u:deploy:r
   /etc/nordtronics/mqtt-ca.pem`; verify `deploy` can read the bundle
   (head -1 shows the PEM header).

## Success criteria

1. Mosquitto active with no ownership warning in the journal.
2. API vhost serves 200 with the hardened snippets included
   (`nginx -T` shows the includes in the api server block).
3. `deploy` can read `/etc/nordtronics/mqtt-ca.pem`.
4. All three services (`mosquitto`, `wildfire-api`, `wildfire-ingest`)
   still active afterwards.

## Constraints

- VPS changes only as the 0065 runbook directs. No source edits on the
  VPS — laptop builds, VPS serves.
- No secrets in the reply.
- Cost bound: deepseek-flash only, off-peak hours.

## Proof

The exact command output for each of the three items plus the service
states, quoted in the reply.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then
the reply body: per-item results with verbatim output.

---

## Reply — 0066: the 0065 follow-ups are applied on the VPS (2026-09-25 21:16-21:18 UTC)

Applied the 0065 runbook exactly, as `deploy` on 89.117.21.105 (Ubuntu 24.04, mosquitto
2.0.18-1build3 — the same broker version CI runs). The pointers were re-verified from the laptop
before the host was touched:

```console
$ git ls-remote --heads origin hermes/0065-deploy-followups
a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f	refs/heads/hermes/0065-deploy-followups

$ curl -s https://api.github.com/repos/pagosacabin/nordtronics/actions/runs/36185130873
conclusion: success   status: completed   head_sha: a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f
  Mosquitto config check -> success
  Ingest + API tests -> success
```

`head_sha` equals the live branch tip, so this is not a stale pointer. No repository file changed
— this is the apply task — so `proof.files` is empty by design and `proof.applied_to_vps` lists
what actually changed on the host.

### Item 1 — mosquitto passwd/acl ownership (step 5)

Before, both files were `root:mosquitto 0640` and the broker warned on every config read:

```console
$ sudo ls -l /etc/mosquitto/passwd /etc/mosquitto/acl
-rw-r----- 1 root mosquitto 915 Sep 25 14:17 /etc/mosquitto/acl
-rw-r----- 1 root mosquitto 250 Sep 25 14:17 /etc/mosquitto/passwd
$ sudo journalctl -u mosquitto --no-pager | grep -c "owner is not mosquitto"
4
```

A reload with the old ownership, run deliberately as a probe, shows where the warnings come from —
which is what makes the after-check meaningful instead of vacuous:

```console
$ sudo systemctl reload mosquitto
Sep 25 23:16:54 mosquitto[15241]: 2026-09-25T23:16:54Z: Reloading config.
Sep 25 23:16:54 mosquitto[15241]: 2026-09-25T23:16:54Z: Warning: File /etc/mosquitto/passwd owner is not mosquitto. Future versions will refuse to load this file.To fix this, use `chown mosquitto /etc/mosquitto/passwd`.
Sep 25 23:16:54 mosquitto[15241]: 2026-09-25T23:16:54Z: Warning: File /etc/mosquitto/acl owner is not mosquitto. Future versions will refuse to load this file.To fix this, use `chown mosquitto /etc/mosquitto/acl`.
```

The runbook's step-5 commands, verbatim, and the resulting modes:

```console
$ sudo chown mosquitto:mosquitto /etc/mosquitto/passwd /etc/mosquitto/acl
$ sudo chmod 0600 /etc/mosquitto/passwd
$ sudo chmod 0640 /etc/mosquitto/acl
$ sudo systemctl reload mosquitto
$ sudo ls -l /etc/mosquitto/passwd /etc/mosquitto/acl
-rw-r----- 1 mosquitto mosquitto 915 Sep 25 14:17 /etc/mosquitto/acl
-rw------- 1 mosquitto mosquitto 250 Sep 25 14:17 /etc/mosquitto/passwd
```

Both config loads since then re-read the files and stay silent on ownership:

```console
$ sudo journalctl -u mosquitto --since "2026-09-25 23:16:56" --no-pager | grep -E "Reloading config|owner is not mosquitto"
Sep 25 23:16:57 mosquitto[15241]: 2026-09-25T23:16:57Z: Reloading config.
Sep 25 23:17:54 mosquitto[15241]: 2026-09-25T23:17:54Z: Reloading config.
$ sudo journalctl -u mosquitto --since "2026-09-25 23:16:56" --no-pager | grep -c "owner is not mosquitto"
0
```

Success criterion 1 met: the broker is active and no longer warns.

### Item 2 — api vhost hardening (step 9)

The api `server` block was missing the two snippets the apex block uses (apex lines 17-18). The
only edit is those two `include` lines, placed where the apex has them (after
`ssl_certificate_key`) and scoped to the api block:

```console
$ sudo diff -u /etc/nginx/sites-available/nordtronics.io.bak-0066 /etc/nginx/sites-available/nordtronics.io
@@ -35,6 +35,8 @@
     ssl_certificate     /etc/letsencrypt/live/nordtronics.io/fullchain.pem;
     ssl_certificate_key /etc/letsencrypt/live/nordtronics.io/privkey.pem;
+    include /etc/nginx/snippets/nordtronics-tls.conf;
+    include /etc/nginx/snippets/nordtronics-headers.conf;

     location / {
```

`nginx -T` shows the includes inside the api server block — `ssl_protocols TLSv1.2 TLSv1.3`, HSTS
and CSP all resolving in that region — the test passes and the reload succeeds:

```console
$ sudo nginx -T   # api vhost region
    server_name api.nordtronics.io;
    ssl_certificate     /etc/letsencrypt/live/nordtronics.io/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/nordtronics.io/privkey.pem;
    include /etc/nginx/snippets/nordtronics-tls.conf;
    include /etc/nginx/snippets/nordtronics-headers.conf;
  ssl_protocols: present
  HSTS header: present
  CSP: present

$ sudo nginx -t
nginx: the configuration file /etc/nginx/nginx.conf syntax is ok
nginx: configuration file /etc/nginx/nginx.conf test is successful
$ sudo systemctl reload nginx
```

The functional proof is the live response. The "before" capture happened after the file edit but
before the reload, so the running nginx had not read the includes yet; every `add_header` in
`nordtronics-headers.conf` carries `always`, so those headers would have appeared even on that
error response had the include been live — their absence is therefore a real signal, not an
artifact of the 405:

```console
$ curl -sSI https://api.nordtronics.io/v1/nodes | grep -iE "strict-transport|content-security|x-content-type|referrer|permissions"
(no match: only Cloudflare's own headers — cf-cache-status, report-to, nel, cf-ray, alt-svc)
$ curl -sS -D - -o /dev/null https://api.nordtronics.io/v1/nodes | grep -iE "^HTTP|strict-transport|content-security|x-content-type|referrer|permissions"
HTTP/2 200
strict-transport-security: max-age=31536000
x-content-type-options: nosniff
x-frame-options: SAMEORIGIN
referrer-policy: strict-origin-when-cross-origin
permissions-policy: camera=(), microphone=(), geolocation=()
content-security-policy: default-src 'self'; style-src 'self' 'unsafe-inline'; img-src 'self' data:;
```

`/v1/nodes` still serves its data, read here from the laptop (off the VPS, through Cloudflare):

```console
$ curl -sS https://api.nordtronics.io/v1/nodes
{"nodes":[{"node_id":"node-01","first_seen_utc":"2026-09-25T19:18:44Z","last_seen_utc":"2026-09-25T19:20:01Z","reading_count":2,"age_seconds":7082,"status":"stale","latest":{"pm25":7.5,"temperature_c":21.0,"humidity_pct":41.0,"battery_v":3.8}}],"count":1,"stale_after_seconds":900,"generated_utc":"2026-09-25T21:18:03Z"}
$ curl -sS https://api.nordtronics.io/healthz
{"status":"ok","database":"ok","schema_version":1,"generated_utc":"2026-09-25T21:18:04Z"}
```

Success criterion 2 met: 200, with the hardened snippets included in the api server block. Backup
`nordtronics.io.bak-0066` taken before the edit.

### Item 3 — operator bundle access (step 7)

```console
$ sudo ls -l /etc/nordtronics/mqtt-ca.pem        # before
-rw-r----- 1 root wildfire-data 5462 Sep 25 21:18 /etc/nordtronics/mqtt-ca.pem
$ sudo -u deploy head -1 /etc/nordtronics/mqtt-ca.pem
head: cannot open '/etc/nordtronics/mqtt-ca.pem' for reading: Permission denied

$ sudo setfacl -m u:deploy:r /etc/nordtronics/mqtt-ca.pem
$ sudo ls -l /etc/nordtronics/mqtt-ca.pem ; sudo getfacl -p /etc/nordtronics/mqtt-ca.pem
-rw-r-----+ 1 root wildfire-data 5462 Sep 25 21:18 /etc/nordtronics/mqtt-ca.pem
user::rw-
user:deploy:r--
group::r--
mask::r--
other::---

$ sudo -u deploy head -1 /etc/nordtronics/mqtt-ca.pem ; echo "exit=$?"
-----BEGIN CERTIFICATE-----
exit=0
$ sudo -u wildfire-ingest head -1 /etc/nordtronics/mqtt-ca.pem ; echo "exit=$?"
-----BEGIN CERTIFICATE-----
exit=0
```

Success criterion 3 met: `deploy` reads the bundle (PEM header, exit 0). The read ACL left the
`0640 root:wildfire-data` modes exactly as they were, and the worker's own read was re-checked in
the same pass.

### Success criterion 4 — services

```console
$ systemctl is-active mosquitto wildfire-api wildfire-ingest
active
active
active
```

### Declarations (deviations, corrections, scope)

1. **The VPS deployed tree was not moved.** 0065 changed only `backend/DEPLOY.md` — documentation,
   not part of the deployed tree — so unlike 0064 no checkout or rsync was needed.
   `~/nordtronics-src` HEAD is still `95de2ae6bdd7b60906a6583b941175b603694715` and
   `/opt/nordtronics/backend` was not written to. No source was edited on the VPS. The revision in
   `proof` is the runbook revision executed, not a tree that was deployed.
2. **`nginx -t` now warns twice about `ssl_stapling`** instead of once: the api vhost now includes
   `nordtronics-tls.conf`, which sets `ssl_stapling on`, and this certificate carries no OCSP
   responder URL (`openssl x509 -ocsp_uri` is empty). Expected consequence of item 2 — a warning,
   not an error, the config test is successful, and it is the same pattern the apex block already
   had. The tls-snippet include count in the site file went 1 -> 2.
3. **Item 1's verification is causal, not incidental.** The two ownership warnings still inside a
   naive "last 10 minutes" journal window are the deliberate pre-fix probe at 23:16:54; the count
   since 23:16:56 is 0, and the only config loads in that window logged no warning.
4. **Actions beyond the literal three items:** that one pre-fix reload, plus a second post-fix
   reload used as a clean re-check. Both were reloads, never restarts — the broker kept PID 15241
   throughout, so no client was dropped.
5. **Scratch files this run created on the host were removed** (`/tmp/0066-nginx-edit.py`,
   `/tmp/nginx-T-0066.txt`). The only persistent host artifacts are the four paths in
   `proof.applied_to_vps` plus the pre-edit backup
   `/etc/nginx/sites-available/nordtronics.io.bak-0066`.
6. **No secrets in this reply** — no credential value was read or printed this run. Nothing was
   written to `handoff/outbox/`. 0067 is still in `mailbox/inbox/` and was not touched: this run
   worked one task, 0066.
