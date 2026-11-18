# 0214 — Crew vault pilot: shared memory for the laptop agents

## Assignment
- Hermes (default profile): set up the vault, seed it, act as librarian, and run the retest by driving Newton via Herdr.
- Newton: retest subject only — answers two questions from the vault in a fresh session.
- Juno: verifies from the staged proof. No Copilot step — this is a notes repo, not code.
- Stephen: no action. Opening the vault folder in Obsidian later is his optional window into it; the pilot does not require the app.

## Context
- On 2026-10-27 Stephen turned down an Obsidian vault: the git mailbox already gave Juno↔Hermes shared state, and a vault would duplicate it. On 2026-11-18 he reversed that for a specific reason: the crew has grown sideways on the laptop — Herdr panes, Newton, multiple Hermes profiles, Copilot — and those agents share no memory with each other. The mailbox is a task queue with proof; it was never a shared knowledge base for the local crew.
- Stephen's direction (2026-11-18): spec a SMALL pilot. The vault is the local crew's durable knowledge — bench facts, decisions, how-we-do-it pages. The mailbox remains the only task queue; nothing in the vault assigns work. Juno stays outside the vault by design (her channel is the mailbox).
- This pilot proves three things: the vault exists with real content, the write conventions hold, and an agent that did none of the setup can answer questions from the vault alone.

## Task
1. Create the vault at `~/crew-vault/` on the laptop. `git init`. If your GitHub login allows it, create a PRIVATE repo `pagosacabin/crew-vault`, add it as remote, and push; if repo creation is blocked, keep the vault local-git only and report that plainly — the pilot is still valid.
2. Write `README.md`: what the vault is, the jobs split (vault = knowledge, mailbox = tasks), and the conventions in the Constraints section below, in plain words. Write `INDEX.md`: a map of the vault's pages, one line each.
3. Seed these pages with real content drawn from records (task files, the repo, your own run notes) — no invented facts; mark anything unverified as unverified:
   - `bench/propane-sensor.md` — the propane bench as wired: ESP32-C3 Super Mini; ADS1115 at 0x48 with VDD 5 V, no dividers, SDA GPIO7 / SCL GPIO6; MQ-6 → A0, Figaro TGS2610-D00 → A1; buzzer GPIO10, LED GPIO5; clean-air baselines (~1.47 V / ~1.52 V); alarm threshold 2.50 V in NVS. Date and source (0208) on the page.
   - `bench/wifi-lessons.md` — what's proven about the house WiFi: Starlink router, app-managed, no MAC-filter UI; one passphrase for both bands (Stephen, 2026-11-18); the propane board's `Auth Expired` refusals; the October lesson that a disconnect reason cannot distinguish a wrong PSK from a WPA3/SAE problem; the presence-sensor C3 (Arduino-framework build) joins fine. Include 0212's outcome if it has landed by the time you write this; otherwise mark the join question OPEN.
   - `decisions/2026-11-18-wifi-provisioning-and-skeleton.md` — Stephen's portal rule (no staged password files; credentials entered on the board via captive portal) and the three-layer ESPHome skeleton (portal / firmware defaults / per-device settings), pointing at `firmware/skeleton/` in the nordtronics repo (0213 — mark "in flight" if it hasn't landed yet).
   - `crew/roles.md` — the crew map: Hermes (hands, mailbox reader, routes Newton via Herdr), Newton (software, headless, never reads the mailbox), Copilot (peer reviewer for GitHub-bound work, driven via Herdr), Herdr/herdr-core (`hc` — the pane manager), Juno (external: specs, verification, memory; reaches this machine only through the mailbox).
   - `log/2026-11-18.md` — first log entry: vault created at Stephen's direction, what was seeded, by whom.
4. Commit as you go, one commit per page or logical step, with the agent named in each commit message (e.g. `hermes: seed propane bench page`).
5. Retest — the proof the vault works as SHARED memory: in a FRESH session, drive Newton via Herdr to answer two questions using ONLY the vault (no task files, no prompting beyond where the vault lives): (a) "On the propane bench, what is the ADS1115's I2C address, and which GPIOs are SDA and SCL?" (b) "What is the rule for WiFi credentials on new boards — what does the portal take, and what must never be put in a file?" Record both questions and Newton's answers verbatim for the reply.
6. Stage the reply with: vault path; remote URL or "local git only"; the vault tree listing; `git log --oneline` output; the retest Q&A verbatim; and any convention that didn't work as written.

## Success criteria
- `~/crew-vault/` exists, git-initialized, containing README, INDEX, the four seed pages, and the first log entry — all committed with agent attribution.
- Newton answers both retest questions correctly from the vault alone, in a fresh session.
- The staged reply carries the full proof listed in step 6.

## Constraints
- Conventions (write them into the README and follow them): any crew agent may APPEND to `log/` (one file per day, entries signed with the agent's name); curated pages (`bench/`, `decisions/`, `crew/`, `INDEX.md`) are edited by Hermes as librarian only — other agents propose changes via log entries; every fact carries a date and a source; commits name the agent.
- NO secrets or credentials in the vault, ever — same rule as the mailbox.
- Pilot scope only: no migrating other memory stores, no Obsidian app install, no mailbox changes, no Juno integration, no changes to any existing repo.
- Seed content comes from records only. If a fact isn't in a record you can reach, leave it out or mark it unverified.
- Cost: standard tier; no premium models.
- If GitHub repo creation is blocked, do not stall the pilot on it — local git plus a plain report is acceptable.

## Proof
- The staged reply itself: vault path, remote status, tree listing, git log, verbatim retest Q&A.

## Reply format
- Staged reply in `mailbox/staged/` per protocol 1.3.0: status, the proof from step 6, notes field with deviations or convention problems.
