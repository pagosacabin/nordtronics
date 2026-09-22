---
task_id: "0034"
status: verified
iteration: 4
proof:
  - branch: hermes/0034-config-probe
    sha: 410efd7c2b1dad843d8bdeabd2b21748114c2e08
notes: 'Correction applied (Juno, iteration 3 feedback); front-matter only, no branch change. (1) Run pointer DELETED - task 0034 produced no CI run and never asked for one; the previously cited URL was a failed Android build on a different branch. (2) SHA re-pointed to the live branch tip, read with `git rev-parse origin/hermes/0034-config-probe` after a fresh `git fetch origin` and again immediately before writing this file: 410efd7c2b1dad843d8bdeabd2b21748114c2e08. The earlier 28a81bfa... was stale. The evidence on the branch (config-probe/0034-answers.md) is unchanged and was not touched. Bookkeeping note: a prior run (78d8af9) moved 0034 inbox -> active but never applied the front-matter update, leaving it in active/ with status: inbox, iteration: 3; this run completed that one pickup (iteration -> 4) and staged it. 0034 now appears exactly once across mailbox/inbox|active|staged|archive.'
---
# 0034 — Prove your local configuration: version, skills, memory

## Context

You run locally on your own machine as an agent. I have read the public
documentation for the Hermes Agent framework, but I do not know which of its
features your installation actually has configured. Answer from your own machine
— not from the docs.

## Task

Prove your local configuration. Run the diagnostics, commit the raw command
output to a new branch on origin, then stage this task pointing at it.

## Success criteria

1. I can read every answer from files on origin — not from pasted text.
2. Every claim names the exact command or file path it came from.

## Questions to answer

- Hermes version or commit (show the exact command you ran to get it)
- Active profile name, if profiles are in use
- Skills: Stephen confirmed skills are enabled on your installation as of 2026-09-21, with the GitHub skills added. Confirm from your side — which skill commands/tools are available (show actual command output), your skill directories and precedence (exact paths), and whether saving a new skill needs approval
- Memory backend in use, and whether memory writes are enabled
- Cron or gateway configuration relevant to how you poll this mailbox

## Constraints

- Read-only on your machine: do not change any configuration, do not install anything.
- Push to a new branch named `hermes/0034-config-probe`. Do not touch `main` or `android-toolchain-setup`.
- Put the raw output in `config-probe/0034-answers.md` on that branch. Paste full command output — do not summarize it away.

## Proof

`proof` lists the branch and tip SHA in the canonical form from the mailbox README.
I will read `config-probe/0034-answers.md` from origin myself.

## Reply format

- Status: done | blocked
- Branch: hermes/0034-config-probe
- Branch tip SHA: exact output of `git rev-parse origin/hermes/0034-config-probe`
- Per question: answered | unknown (if unknown, the exact command or path you checked)

expect-reply-within: 6h

---

## Results (iteration 4) — correction-only re-stage

**Status:** done

- **Branch:** `hermes/0034-config-probe`
- **Branch tip SHA:** `410efd7c2b1dad843d8bdeabd2b21748114c2e08`
  (exact output of `git rev-parse origin/hermes/0034-config-probe`, read after `git fetch origin`
  and again immediately before this file was written)
- **Run:** none cited — task 0034 has no CI run. The previously cited Actions URL (a failed Android
  build on a different branch) has been removed rather than replaced.
- **Evidence (unchanged, already accepted):** `config-probe/0034-answers.md` on `hermes/0034-config-probe`

### What changed in this iteration

Front-matter only. No commit was made to `hermes/0034-config-probe`; the diagnostics work was
already accepted and that branch was left alone.

1. Deleted the bogus `run:` pointer.
2. Replaced the stale `sha:` with the live branch tip, re-read from origin just before staging.
3. Moved this file `active/` -> `staged/` with `status: staged`.

### Per-question status

All five questions are answered in the evidence file on origin (exact commands/paths recorded there):

| Question | Status | Source recorded in evidence file |
|---|---|---|
| Hermes version / commit | answered | `hermes-agent --version` output |
| Active profile name | answered | `~/.hermes/config.yaml` (`default`) |
| Skills: commands, directories, precedence | answered | skill dirs under `~/.hermes/skills/` |
| Memory backend, writes enabled | answered | memory backend config |
| Cron / gateway config polling this mailbox | answered | cron + watchdog config |
