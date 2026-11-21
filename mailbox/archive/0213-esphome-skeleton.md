---
task_id: "0213"
protocol_version: 1.3.0
status: verified
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-18T20:13:41+00:00"
lease_until: "2026-11-18T21:43:41+00:00"
proof:
  branch: hermes/0213-esphome-skeleton
  sha: 52aafcf241712a7077115c1004753dfacf71113d
  run: https://github.com/pagosacabin/nordtronics/actions/runs/44894124925
  pr: https://github.com/pagosacabin/nordtronics/pull/7
  review: >-
    PR #7 review comments, relayed verbatim from herdr-core pane t17
    (copilot-main): issuecomment-7061204070 (pass 1, FIX),
    issuecomment-7061204363 (pass 2, FIX), issuecomment-7061204677
    (passes 3-5, final verdict SHIP at 52aafcf)
  files:
    - .gitignore
    - firmware/skeleton/esphome-base.yaml
    - firmware/skeleton/README.md
    - firmware/skeleton/secrets.yaml.example
    - firmware/skeleton/BUILD-AND-PROVISION.md
    - firmware/skeleton/check-secrets.sh
  ntfy: >-
    not applicable. This task's proof section names branch SHA, PR URL, compile
    excerpt and review pointer; it names no build topic, and no artifact was
    published, so there is no receipt id to record.
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
  note: "This harness exposes no per-call usage to the cron session; token and cost fields are unknown rather than estimated."
notes: |
  RESUMED AND COMPLETED 2026-11-21 03:15-03:30 MST (10:15-10:30 UTC), off-peak
  (PEAK: OFF-PEAK 10:15 UTC), PROTOCOL MATCH 1.3.0. iteration stayed 1: this is a
  resume of the parked task, not a new pickup (the 2026-11-18 run claimed it).

  BOTH BLOCKERS FROM 2026-11-18 ARE CLEARED, and I checked rather than assumed.
  PR write: the invalid-head probe now answers 422 Validation Failed (head
  invalid) instead of the old 403 "Resource not accessible by personal access
  token" — per the probe's own discriminator that means the grant is present.
  Copilot: herdr-core pane t17 (copilot-main, live, turn 142+) is signed in and
  reviewed this branch across five passes. The gh keyring login is still dead
  ("The token in default is invalid"), so the PAT is the credential in use.

  WHAT WAS DONE THIS RUN. Opened PR #7, drove five Copilot review passes, fixed
  all six findings, re-verified config+compile at the final tip, and staged.

  VERIFICATION AT THE STAGED TIP 52aafcf (worktree rebuilt from
  origin/hermes/0213-esphome-skeleton, secrets.yaml.example copied to
  secrets.yaml): esphome 2026.9.1; `esphome config esphome-base.yaml` ->
  "INFO Configuration is valid!"; `esphome compile esphome-base.yaml` ->
  "INFO Successfully compiled program." (RAM 32.2 % = 103,608 / 321,296 B;
  Flash 48.9 % = 897,886 / 1,835,008 B; config_hash=0xaa514391).
  `git ls-remote --heads origin hermes/0213-esphome-skeleton` = 52aafcf, and
  run 44894124925 has headSha 52aafcf with conclusion success, so the proof
  pointers are the branch tip and not a stale run.

  REVIEW OUTCOME. Copilot found six issues in five passes and the FINAL verdict
  is SHIP at 52aafcf ("No findings"). Pass 1: the example API key was usable
  (major) and the setup-AP default is public (minor). Pass 2: the documented
  workflow could not run the guard for a new project (major) and base64
  validation was permissive (minor). Pass 3: the guard normalised whitespace
  before validating. Pass 4: a quoted value with an inline comment was misread.
  Every one is fixed, not argued: the guard is firmware/skeleton/check-secrets.sh
  and it refuses a missing secrets file (exit 2), the committed example key
  (exit 3) and any key that is not canonical 32-byte base64 with no inner
  whitespace (exit 4), warning while the bench setup-AP default is in place.

  ONE DEVIATION FROM A REVIEW FINDING, DECLARED. Copilot asked for a placeholder
  that FAILS validation until replaced. This task's first success criterion is
  the opposite: "esphome config validates the skeleton as committed (with the
  example secrets)". I implemented the intent instead of the literal suggestion,
  so the example key stays valid-format and the guard makes it fail closed at
  the only step that matters — before a flash. Both README step 5 and
  BUILD-AND-PROVISION run the guard in the documented path; measured cases at
  the staged tip are listed in the reply body below.

  SCOPE NOTE FOR THE VERIFIER. The branch also carries the
  firmware/skeleton/BUILD-AND-PROVISION.md addendum and its two correction
  commits from task 0216 (documentation only, no code), so PR #7 is six commits
  above main, not three. .gitignore gains only "secrets.yaml"; the .esphome/
  build tree is still uncovered by the root .gitignore (observed, deliberately
  not changed in this task).

  NOT DONE, ON PURPOSE. No push to main, no merge, no skill written (the spec
  says the skill is a separate follow-on task), and firmware/propane-sensor/ was
  not touched.
---
# 0213 — ESPHome device skeleton: three-layer base for every ESP32 build

## Assignment
- Hermes: build the skeleton and README on a branch in the nordtronics repo; drive the Copilot peer review via Herdr.
- Copilot: peer reviewer for this GitHub-bound work (reviewer only — Juno still verifies independently).
- Juno: verifies the branch, files, review, and compile proof; then files the follow-on skill task.
- Stephen: no action in this task. He provisions the first real board through the portal when the skeleton gets its first use.

## Context
- On 2026-11-18 the propane bench build (0208–0212) burned a morning on a staged-password hunt: WiFi credentials typed into a file, read by Hermes, flashed as secrets, ~190 failed join attempts before the cause was cornered. Stephen's call, same day: captive-portal provisioning from now on, and a reusable ESP32/ESPHome skeleton so every build starts from the same proven base instead of a blank page.
- This generalizes Stephen's 2026-10-03 wildfire rule: the portal configures WiFi credentials (and node settings there); infrastructure addresses are firmware defaults, never portal fields.
- Reference builds: `firmware/propane-sensor/propane-bench.yaml` (0208, ESPHome 2026.9.1) and the November C4002 presence build. Target the current ESPHome defaults.
- Stephen also wants this to become a skill Hermes holds (his words: "this needs to be a skill that Hermes learns"). That is a SEPARATE follow-on task Juno files after this skeleton is verified — per the teaching ladder, he encodes only verified procedures. Do not write the skill in this task.

## Task
Create `firmware/skeleton/` in the nordtronics repo with two files:

1. `firmware/skeleton/esphome-base.yaml` — the skeleton, organized in three clearly commented layers:
   - **Substitutions header** at the top: `device_name`, `friendly_name`, `board` (default `esp32-c3-devkitm-1`; comment noting S3 boards swap this line). Everything project-specific is added below the skeleton, never inside it.
   - **Layer 1 — Portal provisioning:** `wifi:` with a fallback AP plus the `captive_portal:` component. NO station credentials baked in — no house SSID/password in the config or any committed secrets file. The fallback AP gets its own setup password, defined in substitutions, plainly labeled in a comment as guarding only the setup page (not a real secret).
   - **Layer 2 — Firmware defaults (identical on every board):** native `api:` with encryption (the key is a per-device value supplied from a local, uncommitted secrets file at flash time — the YAML references it via `!secret`; commit only a `secrets.yaml.example` with placeholder values), `ota:` (ESPHome platform), `logger:`, SNTP `time:`, and the standard diagnostics: uptime sensor, WiFi signal sensor, IP address text sensor, restart button. Set the `esphome:` project name/version (nordtronics skeleton v1).
   - **Layer 3 — Per-device settings pattern:** a marked section with ONE worked example — a persisted template `number` with `restore_value: true` standing in as an example threshold — plus a comment block stating the rule: each project declares its own settings here (thresholds, offsets, intervals, toggles); all persisted on the board, all adjustable from Home Assistant after adoption. No other example entities.
2. `firmware/skeleton/README.md` — short: the three layers and why; how to start a new device (copy the base, set substitutions, add project code below the skeleton, generate the per-device API key into a local secrets file, flash, provision WiFi at the portal — the steps Stephen does on his phone — then adopt in Home Assistant); the rule that house credentials are never committed or staged in files again.

Open a PR with the branch and complete the Copilot peer-review step before staging the reply; address review comments or note why not in the reply.

## Success criteria
- `esphome config` validates the skeleton as committed (with the example secrets) and `esphome compile` succeeds for the default C3 target. Report the ESPHome version used and the compile result.
- No house WiFi credentials anywhere in the committed files; the only passwords present are the labeled setup-AP password and placeholder secrets.
- Copilot peer review completed, with its pointer in the reply.
- PR opened against main.

## Constraints
- Keep v1 minimal: no project sensors, no MQTT, no web_server, no example code beyond the one settings-pattern number. Start small, prove the base, extend later.
- Do NOT touch the propane board or `firmware/propane-sensor/` in this task (0212 is resolving that board separately). Do NOT retrofit the presence sensor.
- No real secrets in the repo, the task reply, or logs quoted in the reply.
- Cost: standard tier for build work; no premium models.

## Proof
- Branch name + head SHA on origin, PR URL, ESPHome compile output excerpt (version + success line), Copilot review pointer.

## Reply format
- Staged reply in `mailbox/staged/` per protocol 1.3.0: status, the proof pointers, notes field with anything Juno should know (deviations, review outcome).

---

## Reply — 2026-11-18 13:45 MST (blocked, no proof block)

**Status: blocked, parked in `active/`.** Two of the four success criteria cannot be met on
this host right now, and both are decisions I do not own. There is deliberately no `proof`
block and nothing was staged.

### What is built and verified

Branch `hermes/0213-esphome-skeleton`, tip **`668f244`** on origin (two commits above main
`20ce1bd`):

| Commit | Contents |
|---|---|
| `c7eeee9` | `firmware/skeleton/esphome-base.yaml`, `README.md`, `secrets.yaml.example`, `.gitignore` (`secrets.yaml`) |
| `668f244` | fix: valid-format example key + README key-generation step |

Verification performed this run, against the branch content:

- ESPHome **2026.9.1** (`esphome version`).
- `esphome config esphome-base.yaml` with `secrets.yaml.example` copied to `secrets.yaml`
  → `INFO Configuration is valid!`
- `esphome compile esphome-base.yaml` (default `esp32-c3-devkitm-1`, esp-idf framework)
  → `INFO Successfully compiled program.` in 29.6 s; `change-me.bin` 897,744 B, 51 % of the
  app partition free; total image 897,886 B (Flash 48.9 %).
- Credential scan of `firmware/skeleton/`: the only password is the labelled setup-AP
  substitution (`provision-me`, comment states it guards only the setup page); no SSID, PSK
  or base64 blob other than the fake 32-byte placeholder key.

### Defects found and fixed before pushing

The previous tick died with the files staged and never committed. Its draft failed the first
criterion as written: with `secrets.yaml.example` copied to `secrets.yaml`, `esphome config`
rejected the skeleton with

```
api: [source esphome-base.yaml:69]
  encryption:
    Invalid key format, please check it's using base64.
```

The placeholder `REPLACE_WITH_32_BYTE_BASE64_KEY` was not base64. It is now a valid-format
32-byte placeholder (`cGxhY2Vob2xkZXItcmVwbGFjZS1tZS13aXRoLXJlYWw=` = the words
"placeholder-replace-me-with-real"), documented as unusable, and the README's nonexistent
`esphome secrets.yaml` step is replaced with the `python3 -c "…os.urandom(32)…"` command.

### Blocker 1 — no PR can be opened

```
$ gh api -X POST repos/pagosacabin/nordtronics/pulls     -f title=probe -f head=probe-does-not-exist -f base=main
{"message":"Resource not accessible by personal access token","status":"403"}
```

A deliberately invalid head ref returns 422 when the grant is present, so 403 means the
credential itself lacks pull-request write. `repos/… --jq .permissions` reports
`{"admin":true,…}` — that is the account's repo role, not the fine-grained PAT's grant.
The gh keyring login is separately dead (`The token in default is invalid`), so there is no
second credential to fall back to.

**Decision owed:** add **Pull requests: Read and write** to the fine-grained PAT in the
cronrunner `.env` (plus Issues: write if the review should land as a PR comment), or
re-authenticate `gh` with an OAuth token carrying `repo`.

### Blocker 2 — Copilot peer review unavailable

herdr-core `t17` (`copilot-main`, live, idle, turn 88) answers
`Please use /login to sign in to use Copilot`. With no PR there is also nowhere to post a
review.

**Decision owed:** log the Copilot CLI in, or name another peer reviewer for 0213.

### What I did not do

No direct push to `main`, no merge, no substitute reviewer, no invented review text. The
skeleton is on the branch and nothing else moved.

### Cost

Not reportable: this profile does not expose per-call API usage to the harness, so token and
cost fields are `unknown` rather than estimated. Rate window was off-peak (`PEAK: OFF-PEAK
20:45 UTC`).

## Reply — 2026-11-21 03:30 MST (staged)

**Status: done.** Branch `hermes/0213-esphome-skeleton` @ **`52aafcf`** on origin, PR
**https://github.com/pagosacabin/nordtronics/pull/7** open against `main` and MERGEABLE,
Copilot peer review **SHIP**.

### Why this is no longer blocked

The 2026-11-18 reply parked two blockers; both were re-checked this run rather than assumed:

| Blocker then | State now (probe) |
|---|---|
| No PR can be opened — invalid-head probe answered `403 Resource not accessible by personal access token` | `422 Validation Failed (head invalid)` — by the same probe's own discriminator the grant is present, and PR #7 now exists |
| Copilot peer review unavailable — pane `t17` answered "Please use /login" | `t17` (`copilot-main`) is signed in and ran five review passes on this branch |

The `gh` keyring login is still invalid, so the cronrunner fine-grained PAT is the
credential in use.

### What the branch contains

`firmware/skeleton/` — the three-layer base plus its documentation and a pre-flash guard:

| File | Contents |
|---|---|
| `esphome-base.yaml` | substitutions header; layer 1 portal provisioning (`wifi.ap` + `captive_portal`, no station credentials); layer 2 firmware defaults (encrypted `api`, `ota`, `logger` on USB-SERIAL/JTAG, SNTP, diagnostics, project stamp); layer 3 the one worked `number` example with the settings rule |
| `README.md` | the three layers and the start-a-new-device walkthrough, including the guard step |
| `secrets.yaml.example` | placeholder key only; `secrets.yaml` is gitignored |
| `check-secrets.sh` | pre-flash guard (see below) |
| `BUILD-AND-PROVISION.md` | the verified build-and-provision path — carried over from task 0216 in three documentation-only commits |
| `.gitignore` | adds `secrets.yaml` |

### Success criteria

| Criterion | Evidence at `52aafcf` |
|---|---|
| `esphome config` validates as committed, `esphome compile` succeeds for the C3 target | ESPHome 2026.9.1; `INFO Configuration is valid!`; `INFO Successfully compiled program.` — RAM 32.2 % (103,608 / 321,296 B), Flash 48.9 % (897,886 / 1,835,008 B), `config_hash=0xaa514391` |
| No house WiFi credentials in the committed files; only the labelled setup-AP password and placeholder secrets | `git grep` over the tree: the only password is `setup_ap_password: "provision-me"`, labelled bench-only; the only base64 blob is the documented placeholder; no SSID or PSK |
| Copilot peer review completed, pointer in the reply | PR #7 comments `#issuecomment-7061204070`, `#issuecomment-7061204363`, `#issuecomment-7061204677`; final verdict **SHIP** at `52aafcf` |
| PR opened against main | PR #7, base `main`, head `52aafcf`, MERGEABLE, `html-link-check` pass (run 44894124925, headSha = `52aafcf`) |

### The review, and the guard it produced

Six findings across five passes, all fixed:

| Pass | Severity | Finding | Fix |
|---|---|---|---|
| 1 | major | the committed example API key decodes to 32 bytes, so it is *usable* — every board flashed with it would share one public key | `check-secrets.sh` refuses the placeholder (exit 3) |
| 1 | minor | the default setup-AP password is public and shared | comment + README rule: bench-only, unique per board before a device leaves the bench; the guard warns while `provision-me` is in place |
| 2 | major | the documented workflow cannot run the guard for a new project — the script lives only in the skeleton | docs invoke the skeleton's guard with the project's own two files |
| 2 | minor | base64 validation was permissive (`base64 -d` decodes a valid prefix followed by junk) | strict canonical check: `b64decode(validate=True)` plus a re-encode equality test |
| 3 | minor | the guard stripped whitespace before validating, so a key with inner whitespace was accepted | quoted values are read verbatim |
| 4 | minor | a quoted value with an inline `# comment` was misread and refused | both quoted patterns accept an optional trailing comment |

Measured behaviour of the final guard (each case run at `52aafcf`): quoted canonical + comment
`rc=0`; quoted trailing space `rc=4`; quoted inner space `rc=4`; unquoted + comment `rc=0`;
single-quoted + comment `rc=0`; missing file `rc=2`; example placeholder `rc=3`; 31-byte key
`rc=4`; `setup_ap_password: "provision-me"` with a comment → bench-default warning; a project's
own AP password → no warning.

**Deviation from one finding, declared.** Copilot asked for an example key that *fails*
validation. This task's first criterion requires `esphome config` to validate the skeleton **as
committed (with the example secrets)**, which such a key would break. I implemented the finding's
intent — the default fails closed at the only step that matters — instead of its literal text, and
made the guard mandatory in both documented paths.

### Not done, on purpose

No push to `main`, no merge, no skill written (the spec makes the skill a separate follow-on
task), and `firmware/propane-sensor/` untouched. Nothing was published to ntfy: the proof section
names branch SHA, PR and compile excerpt, not a build topic.


## Juno closure — 2026-11-21

Verified in full by Juno 2026-11-21: branch hermes/0213-esphome-skeleton @ 52aafcf241712a7077115c1004753dfacf71113d matched origin; ESPHome 2026.9.1 config valid, compile succeeds (RAM 32.2%, Flash 48.9%); Copilot five passes, six findings fixed, final verdict SHIP; check-secrets.sh guard measured at every exit code. The one declared deviation (example key stays valid-format per the task's first success criterion; the guard fails closed before flash) is accepted as the right trade.

Merged to main 2026-11-21 on Stephen's word ("Merge this one, keep asking"): PR #7, merge commit 4ac7a14154a5f78850eb7bbb721914f47fb0ab23. Task closed.
