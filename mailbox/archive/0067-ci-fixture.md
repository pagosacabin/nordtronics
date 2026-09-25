---
task_id: "0067"
protocol_version: 1.0.0
status: verified
iteration: 1
expect-reply-within: 6h
proof:
  - branch: hermes/0067-ci-mosquitto-ownership
    sha: 1594b5e28d525a342a375c1e46854eef293699c5
  - run: https://github.com/pagosacabin/nordtronics/actions/runs/36196208715
  - files:
      - backend/mosquitto/test-fixture.sh
      - backend/mosquitto/smoke-test.sh
      - .github/workflows/backend.yml
notes: |
  Done on `hermes/0067-ci-mosquitto-ownership`, cut from the 0065 tip
  a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f as directed. Three files,
  72 insertions / 4 deletions, one commit 1594b5e. CI green at the branch tip
  (run 36196208715, both jobs success, headSha == tip). Zero VPS changes.

  The fixture change is the spec's, verbatim: passwd is created and then handed
  to `mosquitto:mosquitto` 0600, acl to `mosquitto:mosquitto` 0640, the
  ownership/modes DEPLOY.md step 5 requires. The CI runner has the `mosquitto`
  user (the package creates it), so nothing had to be faked; the fixture now
  fails loudly if that user is ever absent instead of quietly falling back to
  root-owned files.

  Declared scope extension (2 files beyond the spec's test-fixture.sh, and the
  reason the task's criterion 2 needs them): the fixture change alone cannot make
  CI guard the rule. With the files correctly owned the broker simply does not
  warn, and 0065's own declared gap was that nothing asserts the warning — so a
  green run after the fixture edit would still pass if the ownership regressed
  tomorrow. smoke-test.sh therefore asserts the rule against the broker it
  starts: no "owner is not mosquitto" line in that broker's log, and the
  owner/group/mode of the two paths the config under test names. The modes need
  the second assertion because the broker only complains about ownership, so a
  fixture with the right owner and passwd at 0640 passes the warning check while
  the runbook rule is still broken. backend.yml adds the negative control: it
  puts the pre-0067 fixture back, starts the broker and requires both warnings,
  so the assertion is proven non-vacuous on every run rather than by argument.
  Both smoke-test.sh additions are read-only (it reads its own broker's log and
  stats the files its config names), so the script stays safe to run on the VPS
  — where it now also fails if the rule regresses.

  Non-vacuity, three probes (replica of the CI job in podman `ubuntu:24.04`,
  mosquitto 2.0.18 — the same version as the runner and the VPS — with the
  repo's own scripts, as root, on an ACL-capable filesystem):
  1. pre-0067 fixture (root:mosquitto 0640) -> the new check fails: "FAIL the
     broker warned that a config file is not owned by mosquitto (DEPLOY.md
     step 5)", with those two warning lines in the broker log.
  2. right owner, passwd 0640 -> warning check passes, mode check fails:
     "FAIL password_file /etc/mosquitto/passwd is 'mosquitto:mosquitto 640';
     DEPLOY.md step 5 requires 'mosquitto:mosquitto 600'".
  3. the negative control itself (rule broken, broker started): both
     "Warning: File /etc/mosquitto/{passwd,acl} owner is not mosquitto…" lines
     present. The same two lines appear in the CI run's negative-control step.

  Verification of the shipped pointers, this run: `git ls-remote --heads origin
  hermes/0067-ci-mosquitto-ownership` = 1594b5e28d525a342a375c1e46854eef293699c5;
  `gh run view 36196208715 --json headSha,conclusion` = success @ 1594b5e…; both
  jobs success. Read, not assumed: the job log and the downloaded
  `mosquitto-config-check` artifact (both quoted in the reply) show the layout
  the guard asserts — passwd mosquitto:mosquitto 600, acl 640, no ownership
  warning — and the control step's two warnings.

  WHERE THE ASSERTED VALUES COME FROM (read-only grounding): `ls -l
  /etc/mosquitto/` on the VPS this run gives `-rw------- mosquitto mosquitto
  passwd` and `-rw-r----- mosquitto mosquitto acl` — the values the new mode
  assertion requires are the ones 0066 actually applied, so the check is true on
  the deployed host, not just in CI. The same command shows
  `nordtronics.conf` at root:mosquitto 0640, which is why the config file is not
  part of the assertion (mosquitto does not warn about it). Three read-only ssh
  commands; nothing written, no service restarted, no checkout or rsync (this is
  a branch-only task and DEPLOY.md runbook revision 0065 was not re-executed).

  No ntfy publish: not a build task (no CI-built artifact) and the spec names no
  topic — the same handling as 0061/0065. One correction to declare: my first cut
  of the mode assertion compared 'mosquitto:mosquitto:600' against stat's
  'mosquitto:mosquitto 600' and failed on a *correct* fixture; the podman replica
  caught it before the push. No secrets in this reply; nothing written to
  handoff/outbox/; 0059/0062/0063/0065/0066 untouched.
---

# 0067 — CI fixture: guard the mosquitto passwd/acl ownership rule (branch only)

## Context

0065's notes declared one gap: CI does not exercise the new mosquitto
passwd/acl ownership rule — `backend/mosquitto/test-fixture.sh` still
creates the fixtures as `root:mosquitto` 0640, so the CI broker probe
builds them the old way and the warning is never asserted. The CI broker
is mosquitto 2.0.18, the same version that emits the warning on the VPS.
Branch from the 0065 tip: `hermes/0065-deploy-followups @
a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f`. Name the new branch
`hermes/0067-ci-mosquitto-ownership`.

## Task

On the new branch, change `backend/mosquitto/test-fixture.sh` so the
passwd/acl fixtures are created as `mosquitto:mosquitto`, mode 0600 for
passwd and 0640 for acl — matching the runbook rule 0065 added. If the
CI environment cannot chown to the mosquitto user, say so in the reply
instead of faking it; do not weaken the assertion to pass.

Push the branch and get CI green.

## Success criteria

1. CI is green on `hermes/0067-ci-mosquitto-ownership`.
2. The fixture creates passwd/acl with the runbook's ownership and
   modes, so CI guards the rule.

## Constraints

- Branch only: zero VPS changes.
- Cost bound: deepseek-flash only, off-peak hours.

## Proof

Branch SHA on origin + Actions run URL showing green, both quoted in the
reply.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: the branch SHA, the CI run URL and result, and the diff stat.

---

## Reply — 0067: the fixture now mirrors the runbook, and CI fails when it does not

Branch `hermes/0067-ci-mosquitto-ownership @
1594b5e28d525a342a375c1e46854eef293699c5`, cut from the 0065 tip
`a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f` as directed. CI run 36196208715 =
**success** (`Mosquitto config check`, `Ingest + API tests`). Diff stat:

```console
$ git diff --stat a4b1724..1594b5e
 .github/workflows/backend.yml     | 26 ++++++++++++++++++++++++++
 backend/mosquitto/smoke-test.sh   | 26 ++++++++++++++++++++++++++
 backend/mosquitto/test-fixture.sh | 24 ++++++++++++++++++++----
 3 files changed, 72 insertions(+), 4 deletions(-)
```

Zero VPS changes of any kind.

### 1 — the fixture creates the files the way DEPLOY.md step 5 does

`backend/mosquitto/test-fixture.sh` now ends the passwd/acl steps with the
runbook's owner and modes (was `root:mosquitto 0640` for both):

```bash
chown mosquitto:mosquitto "$MOSQUITTO_DIR/passwd"
chmod 0600 "$MOSQUITTO_DIR/passwd"
...
chown mosquitto:mosquitto "$MOSQUITTO_DIR/acl"
chmod 0640 "$MOSQUITTO_DIR/acl"
```

The CI environment **can** chown to the broker's user — the `mosquitto` package
installs it and the workflow installs the package — so nothing is faked or
weakened. The fixture additionally refuses to run on a host without that user,
so a runner that lost the package fails loudly instead of silently producing
root-owned fixtures and a passing job:

```bash
id -u mosquitto >/dev/null 2>&1 || {
  echo "test-fixture.sh: no 'mosquitto' user on this host — install the mosquitto package (it creates the user and group); the passwd/acl ownership rule cannot be mirrored" >&2
  exit 1
}
```

### 2 — what makes CI actually *guard* the rule (declared scope extension)

The fixture change alone does not guard anything: with the files correctly
owned, the broker simply does not warn — and 0065's declared gap *was* that the
warning is never asserted. So `backend/mosquitto/smoke-test.sh` now asserts the
rule against the broker it starts, two checks after the listener is up:

```console
  ok   broker is listening on 127.0.0.1:8883 over TLS
  ok   the broker loaded password_file and acl_file without an ownership warning
  ok   password_file /etc/mosquitto/passwd is mosquitto:mosquitto 600
  ok   acl_file /etc/mosquitto/acl is mosquitto:mosquitto 640
```

The first reads the log of the broker the script itself started and fails on any
`owner is not mosquitto` line. The second stats the paths the config *under test*
names (`password_file`, `acl_file`), because the broker only complains about
ownership: a fixture with the right owner and passwd at 0640 would otherwise pass
while the runbook rule was broken.

Both are read-only — they read a log and read file metadata — so `smoke-test.sh`
stays safe to run on the VPS, where it now enforces the same rule the runbook
sets. `.github/workflows/backend.yml` adds the negative control as the job's last
step: it re-owns the fixtures the pre-0067 way, starts the broker and requires
the warnings, so the assertion is proven non-vacuous on every run. From the run
above:

```console
--- control broker log ---
2026-09-25T22:20:41Z: mosquitto version 2.0.18 starting
2026-09-25T22:20:41Z: Config loaded from /etc/mosquitto/nordtronics.conf.
2026-09-25T22:20:41Z: Warning: File /etc/mosquitto/passwd owner is not mosquitto. Future versions will refuse to load this file.To fix this, use `chown mosquitto /etc/mosquitto/passwd`.
2026-09-25T22:20:41Z: Warning: File /etc/mosquitto/acl owner is not mosquitto. Future versions will refuse to load this file.To fix this, use `chown mosquitto /etc/mosquitto/acl`.
2026-09-25T22:20:41Z: mosquitto version 2.0.18 running
both ownership warnings present with the rule broken: the check is not vacuous
```

### 3 — non-vacuity, and the failure the check produces

Probes 1, 1b and 3 ran in a podman replica of the CI job (`ubuntu:24.04`,
mosquitto 2.0.18 — the same version as the runner and the VPS — the repo's own
scripts, as root):

```console
=== SMOKE with the new fixture (expect 0) ===
  ok   password_file /etc/mosquitto/passwd is mosquitto:mosquitto 600
  ok   acl_file /etc/mosquitto/acl is mosquitto:mosquitto 640
mosquitto config: ALL CHECKS PASSED
SMOKE_RC=0

=== PROBE 1: pre-0067 fixture (root:mosquitto 0640) ===
  ok   broker is listening on 127.0.0.1:8883 over TLS
  FAIL the broker warned that a config file is not owned by mosquitto (DEPLOY.md step 5)
PROBE1_RC=1

=== PROBE 1b: right owner, passwd 0640 ===
  ok   the broker loaded password_file and acl_file without an ownership warning
  FAIL password_file /etc/mosquitto/passwd is 'mosquitto:mosquitto 640'; DEPLOY.md step 5 requires 'mosquitto:mosquitto 600'
PROBE1B_RC=1

=== PROBE 3: rule restored ===
mosquitto config: ALL CHECKS PASSED
PROBE3_RC=0
```

Probe 1's broker log carried exactly the two warning lines quoted in section 2,
so the ownership check is not passing merely because nothing re-read the files.

### 4 — where the asserted values come from

Read-only grounding on the VPS this run (89.117.21.105 over ssh, three commands,
nothing written):

```console
$ ssh deploy@… 'ls -l /etc/mosquitto/'
-rw-r----- 1 mosquitto mosquitto  915 acl
-rw-r----- 1 root      mosquitto 2964 nordtronics.conf
-rw------- 1 mosquitto mosquitto  250 passwd
$ ssh deploy@… 'mosquitto -h 2>&1 | head -1'
mosquitto version 2.0.18
$ ssh deploy@… 'sudo -n journalctl -u mosquitto --since "-14 days" | grep -c "owner is not mosquitto"'
6
```

So the values the mode assertion requires are the ones **0066 actually applied**
(`passwd` 0600, `acl` 0640, both `mosquitto:mosquitto`) — the check is true on
the deployed host, not only in CI — and the warning it looks for is real on this
broker version (the six journal hits all predate 0066's 23:16 fix).
`nordtronics.conf` is deliberately not asserted: it stays `root:mosquitto 0640`
by design (DEPLOY.md step 4) and mosquitto does not warn about it.

### Declarations

- **Scope extension, declared:** the spec's file is `test-fixture.sh`; this
  branch also touches `smoke-test.sh` and `backend.yml`. The reason is criterion
  2 — the fixture edit alone leaves the warning unasserted, so a green run could
  not fail if the ownership regressed. Nothing was weakened to pass: the added
  checks are new assertions, and the fixture's ownership/modes are exactly the
  runbook's.
- **The check's own bug, caught before the push:** the first cut compared
  `mosquitto:mosquitto:600` against `stat`'s `mosquitto:mosquitto 600` and failed
  on a *correct* fixture. The podman replica found it; the shipped version parses
  the expectation into owner/group/mode and compares against `'%U:%G %a'`.
  (Noted because a probe that only ever passed would not have caught it.)
- **No ntfy publish**: no CI-built artifact and the spec names no topic — same
  handling as 0061/0065.
- **No VPS change and no runbook re-execution**: 0065's DEPLOY.md revision was
  not applied here; the only host contact was three read-only reads.
- No secrets in this reply; nothing written to `handoff/outbox/`;
  0059/0062/0063/0065/0066 untouched.

### Verification of the pointers

```console
$ git ls-remote --heads origin hermes/0067-ci-mosquitto-ownership
1594b5e28d525a342a375c1e46854eef293699c5	refs/heads/hermes/0067-ci-mosquitto-ownership
$ gh run view 36196208715 --json headSha,conclusion
{"conclusion":"success","headSha":"1594b5e28d525a342a375c1e46854eef293699c5"}
$ gh run view 36196208715 --json jobs --jq '.jobs[] | "\(.name): \(.conclusion)"'
Mosquitto config check: success
Ingest + API tests: success
$ gh run download 36196208715 -n mosquitto-config-check -D art && grep -n 'ownership\|mosquitto:mosquitto' art/mosquitto-config-check.log
5:  ok   the broker loaded password_file and acl_file without an ownership warning
6:  ok   password_file /etc/mosquitto/passwd is mosquitto:mosquitto 600
7:  ok   acl_file /etc/mosquitto/acl is mosquitto:mosquitto 640
```
