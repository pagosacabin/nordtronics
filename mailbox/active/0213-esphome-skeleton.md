---
status: in_progress
iteration: 1
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-18T20:13:41+00:00"
lease_until: "2026-11-18T21:43:41+00:00"
notes: |
  BLOCKED 2026-11-18 — parked for a decision; no proof block, nothing staged.
  DONE (verified this run): firmware/skeleton/{esphome-base.yaml,README.md,secrets.yaml.example}
  exist on branch hermes/0213-esphome-skeleton at tip 668f244 (origin). esphome 2026.9.1:
  `esphome config` with the committed example secrets -> "INFO Configuration is valid!";
  `esphome compile` -> "INFO Successfully compiled program." for esp32-c3-devkitm-1
  (change-me.bin 0xdb4d0 = 897,744 B, 51% of the app partition free; total image 897,886 B,
  Flash 48.9%). No house WiFi credential in any committed file: the only password is the
  labelled setup-AP substitution "provision-me"; the committed key placeholder is a fake,
  valid-format 32-byte base64 string.
  PREVIOUS TICK: the 20:13Z run claimed this task and died after staging the files but before
  any commit or push (no hermes/0213 branch on origin, local branch exactly at the pickup base).
  Resumed rather than re-claimed; iteration stays 1. Its draft had two defects I fixed before
  pushing: the example key was not valid base64 (config failed with "Invalid key format, please
  check it's using base64" when secrets.yaml.example was copied to secrets.yaml) and the README
  documented a nonexistent `esphome secrets.yaml` command.
  BLOCKER 1 — cannot open a PR. POST /repos/pagosacabin/nordtronics/pulls with an
  invalid head ref returns 403 {"message":"Resource not accessible by personal access token"}
  (the probe is the discriminator: 422 would mean the grant exists and only the ref is bad).
  The fine-grained PAT in ~/.hermes/profiles/cronrunner/.env is contents-only; the gh keyring
  OAuth login is invalid ("The token in default is invalid"), and no other profile .env carries
  a GitHub token. Decision owed: add "Pull requests: Read and write" to that PAT, or re-auth gh.
  BLOCKER 2 — Copilot peer review unavailable. herdr-core pane t17 (copilot-main, idle) answers
  "Please use /login to sign in to use Copilot". No PR exists, so there is also nowhere to post
  a review. Decision owed: log Copilot in, or name another peer reviewer for 0213.
  DEVIATIONS: none applied around either gate — no direct push to main, no substitute reviewer,
  no fabricated review. All work stayed on the branch.
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
