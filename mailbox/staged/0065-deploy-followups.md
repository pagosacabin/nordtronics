---
task_id: "0065"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0065-deploy-followups
    sha: a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36185130873
  - files:
      - backend/DEPLOY.md
notes: |
  All four items done on `hermes/0065-deploy-followups`, cut from the 0063 tip
  (95de2ae6bdd7b60906a6583b941175b603694715) as directed. One file changed:
  backend/DEPLOY.md (69 insertions, 18 deletions). CI green at the branch tip
  (run 36185130873, both jobs success, headSha == tip). Zero VPS changes.

  Branch/sha/run re-verified after the push: `git ls-remote --heads origin
  hermes/0065-deploy-followups` = a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f and
  the run's headSha is the same commit.

  Where the facts came from (read-only grounding, this run): the exact mosquitto
  warning text and the files' current ownership from the VPS journal and
  `ls -l /etc/mosquitto/*`; the include lines from the live
  /etc/nginx/sites-available/nordtronics.io (apex block lines 17-18) and the
  snippets' contents from /etc/nginx/snippets/. Three read-only commands over
  ssh; nothing written, no service restarted.

  Declared consistency edits beyond the four items (same facts restated
  elsewhere in the doc, which would otherwise contradict the new text): the
  paths table rows for acl/passwd, step 4's and step 12's `install ... acl`
  owner, step 12's upgrade-path chown/chmod pair, two troubleshooting rows, the
  step-9 prose file path (sites-available/nordtronics.io, the real file 0064
  recorded) and step 10's bundle comment (it said deploy could not read the
  bundle, which item 4 makes false). Nothing else in the repo was touched.

  Item 1 is implemented inside step 5 (the step that creates passwd and already
  set its mode) rather than as a new numbered step: inserting a number would
  renumber steps 6-12 and break the ~10 in-document "step N" cross-references
  and the references in prior staged replies.

  Follow-up, declared not fixed (spec scoped to DEPLOY.md): CI does not
  exercise the new ownership. backend/mosquitto/test-fixture.sh still creates
  passwd/acl as root:mosquitto 0640, so the CI broker probe builds fixtures the
  old way and the warning is not asserted. The CI broker is mosquitto 2.0.18 —
  the same version that emits the warning on the VPS — so pointing the fixture
  at mosquitto:mosquitto 0600/0640 would make CI guard the rule.

  No ntfy publish: not a build task (no CI-built artifact) and the spec names no
  topic — same handling as 0061. No secrets in this reply. Nothing written to
  handoff/outbox/; 0059/0062/0063/0064 untouched.
---


# 0065 — Deploy follow-ups: mosquitto file ownership, API vhost hardening, runbook doc fixes (branch only)

## Context

0064 (verified, archived) finished the backend deploy. Its reply left
four small follow-ups open. Branch from the 0063 tip:
`hermes/0063-deploy-abc-fix @ 95de2ae6bdd7b60906a6583b941175b603694715`.
Name the new branch `hermes/0065-deploy-followups`.

## Task

On the new branch, make these `backend/DEPLOY.md` changes (no VPS
changes in this task):

1. Mosquitto file ownership: mosquitto warns that
   `/etc/mosquitto/passwd` and `/etc/mosquitto/acl` are not owned by the
   `mosquitto` user, and future versions will refuse to load them. Add a
   runbook step setting ownership to `mosquitto:mosquitto` (mode 0600 for
   passwd, 0640 for acl) and reloading mosquitto.
2. API vhost hardening: the appended api vhost in
   `/etc/nginx/sites-available/nordtronics.io` is missing the
   tls/headers snippets the main site uses. Add the include lines so the
   API vhost matches the site's TLS and header hardening.
3. Runbook step-10 doc fix: the plaintext-1883 check returns "Connection
   refused" when run on the VPS itself (a host reaching its own public IP
   goes via lo, which UFW accepts); the expected timeout output is only
   reachable from an off-host machine. Correct the expected-output note.
4. Operator bundle access (decision, already made): add
   `setfacl -m u:deploy:r /etc/nordtronics/mqtt-ca.pem` to the runbook's
   bundle step, following the 0061 setfacl pattern, so `deploy` can probe
   the bundle directly.

Push the branch and get CI green.

## Success criteria

1. CI is green on `hermes/0065-deploy-followups`.
2. The diff implements exactly the four items above.

## Constraints

- Branch only: zero VPS changes. A follow-up task applies this on the VPS.
- Cost bound: deepseek-flash only, off-peak hours.

## Proof

Branch SHA on origin + Actions run URL showing green, both quoted in the
reply.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: the branch SHA, the CI run URL and result, and the diff stat.

---

## Reply — 0065: the four follow-ups landed on `hermes/0065-deploy-followups`

Branch `hermes/0065-deploy-followups @ a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f`,
CI run 36185130873 = **success** (`Ingest + API tests`, `Mosquitto config check`),
cut from the 0063 tip `95de2ae6bdd7b60906a6583b941175b603694715` as directed. Diff stat:

```console
$ git diff --stat 95de2ae..a4b1724
 backend/DEPLOY.md | 87 +++++++++++++++++++++++++++++++++++++++++++------------
 1 file changed, 69 insertions(+), 18 deletions(-)
```

One file changed, no VPS change of any kind — this is a runbook-only task.

### 1 — mosquitto file ownership

Step 5 now hands both files to the user the broker runs as, quoting the warning
verbatim (read from `journalctl -u mosquitto` on the host this run, mosquitto
2.0.18 — the same version CI runs):

```console
Warning: File /etc/mosquitto/passwd owner is not mosquitto. Future versions will refuse to load this file.To fix this, use `chown mosquitto /etc/mosquitto/passwd`.
Warning: File /etc/mosquitto/acl owner is not mosquitto. Future versions will refuse to load this file.To fix this, use `chown mosquitto /etc/mosquitto/acl`.
```

```bash
sudo chown mosquitto:mosquitto /etc/mosquitto/passwd /etc/mosquitto/acl
sudo chmod 0600 /etc/mosquitto/passwd   # password file: the broker only
sudo chmod 0640 /etc/mosquitto/acl      # ACL: the broker only, group-readable
sudo systemctl reload mosquitto || echo "mosquitto not running yet; step 8 starts it"
```

`reload` is the unit's `ExecReload=/bin/kill -HUP $MAINPID`, and mosquitto
re-reads `password_file` / `acl_file` on SIGHUP, so the reload is what makes the
step usable on a live host (adding a user); on a fresh host the broker is not up
yet, and that is the one case the trailing `|| echo` tolerates — ownership is
already correct before step 8's first start, so a clean deploy never shows the
warning at all. Both files were `root:mosquitto 0640` on the host. The paths
table, step 4's `install ... acl`, step 12's upgrade path and the troubleshooting
table were moved to the same ownership in the same pass so the doc does not
contradict itself.

### 2 — api vhost hardening

Step 9's block now includes the apex site's two snippets:

```nginx
include /etc/nginx/snippets/nordtronics-tls.conf;
include /etc/nginx/snippets/nordtronics-headers.conf;
```

Grounding, read from the live `/etc/nginx/sites-available/nordtronics.io`: the
apex `server` block includes exactly those two paths (lines 17-18 of the file)
and the appended api block had neither. `nordtronics-tls.conf` carries
`ssl_protocols TLSv1.2 TLSv1.3`, the cipher list, `ssl_session_cache`,
`ssl_stapling on` / `ssl_stapling_verify on` + resolver; `nordtronics-headers.conf`
carries `server_tokens off`, HSTS, `X-Content-Type-Options`, `X-Frame-Options`,
`Referrer-Policy`, `Permissions-Policy` and a CSP — so the API now serves the
same TLS policy and header set as the apex instead of nginx's defaults. The
step's prose also names the real file (`/etc/nginx/sites-available/nordtronics.io`,
the path 0064 recorded) and says to take a backup first.

### 3 — step 10's plaintext-1883 note

The old comment asserted a timeout unconditionally. It now states that this is an
off-host check and spells out why the on-host result differs — exactly what 0064
observed:

```console
# off-host: silence, exit 124 after ~5s       (dropped, as intended)
# on-host:  Error: Connection refused, exit 1 (expected here; proves nothing)
```

The public-IP instruction is kept (step 7 pins `mqtt.nordtronics.io` to 127.0.0.1
in `/etc/hosts`) with the mechanism spelled out: a host dialling its own public IP
routes over the loopback interface, which UFW accepts, so the RST comes from the
missing listener before any firewall rule is consulted.

### 4 — operator bundle access

Step 7's bundle block gained the requested ACL, following 0061's named-user
pattern:

```bash
sudo setfacl -m u:deploy:r /etc/nordtronics/mqtt-ca.pem
```

and its confirm block now checks both readers — `sudo -u wildfire-ingest head -1 …`,
`sudo -u deploy head -1 …`, `getfacl -p …` (expects `user:deploy:r--`). A read ACL
is enough and leaves the bundle's `0640 root:wildfire-data` modes — and therefore
the worker's own access — untouched; probed on this laptop on a copy with the same
modes:

```console
$ setfacl -m u:nobody:r f          # f was -rw-r----- before
$ ls -l f ; getfacl -p f
-rw-r-----+ 1 astroboy astroboy 5 Sep 25 14:19 f
user::rw-
user:nobody:r--
group::r--
mask::r--
other::---
```

The probe ran as my own user on a file I own (this host has no passwordless sudo),
so it proves the ACL semantics and that the mode bits stay 0640 — not a `deploy`
read on the VPS. That read is what the apply follow-up will exercise. Step 10's
operator comment was corrected in the same pass: it claimed `deploy` cannot read
the bundle, which item 4 makes false.

### Declarations (deviations, corrections, scope)

- **Extra edits are consistency edits, not new scope.** Named so the diff is not
  read as more than the four items: the paths-table rows, step 4's and step 12's
  `install -o mosquitto -g mosquitto … acl`, step 12's `chown`/`chmod` pair for
  pre-existing installs, two troubleshooting rows, step 9's file path, step 10's
  bundle comment. Each restates a fact that items 1, 2 and 4 change; leaving them
  would have left the doc contradicting itself. No other file in the repo changed.
- **Item 1 is implemented inside step 5**, the step that creates `passwd` and
  already set its mode, rather than as a new numbered step: inserting a number
  would renumber steps 6-12, which are referenced ~10 times in the doc and in
  earlier staged replies.
- **CI does not exercise the ownership rule.** `backend/mosquitto/test-fixture.sh`
  still creates its fixtures `root:mosquitto 0640`, so the green run proves the
  branch builds and the broker still starts, not that the new ownership silences
  the warning. Declared as a follow-up, not fixed here (the spec's scope is
  DEPLOY.md); the CI broker is mosquitto 2.0.18, the same version that warns, so
  pointing the fixture at `mosquitto:mosquitto` would make CI guard the rule.
- **No ntfy publish**: not a build task (no CI-built artifact) and the spec names
  no topic — the same handling as 0061.
- **Grounding was read-only**: three ssh read commands (nginx site file + the two
  snippets, mosquitto ownership + journal, `sudo -n true`). Nothing was written on
  the host, no service was restarted, and this reply claims no `applied_to_vps`.
- No secrets in this reply; nothing written to `handoff/outbox/`; 0059/0062/0063/
  0064 untouched.

### Verification of the pointers

```console
$ git ls-remote --heads origin hermes/0065-deploy-followups
a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f	refs/heads/hermes/0065-deploy-followups
$ gh run view 36185130873 --json headSha,conclusion
{"conclusion":"success","headSha":"a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f","status":"completed"}
$ gh run view 36185130873 --json jobs --jq '.jobs[] | "\(.name): \(.conclusion)"'
Mosquitto config check: success
Ingest + API tests: success
```
