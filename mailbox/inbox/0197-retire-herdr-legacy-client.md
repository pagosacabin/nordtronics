# 0197 — Cleanup: inventory the dead herdr 0.9.5 client and retire it if nothing depends on it

## Assignment
- **Hermes** (host-local work on your own machine): all steps.
- **Newton:** not used. **Copilot:** not used — no GitHub deliverable; host-local cleanup.
- **Juno:** reads the staged report, verifies the before/after `hc` state matches, archives.

## Context
From 0193's inventory (verified on this machine, 2026-11-16): the live pane manager is **herdr-core 0.1.21** (`hc`, pid 5079, up since 2026-11-13 08:02 MST, 6 live terminals). The older **`herdr` 0.9.5** client's server has been down since its log ended 2026-11-13 03:18 — the morning herdr-core took over. The client binary still sits on the machine (0193 read strings from `~/.local/bin/herdr`) and still answers questions: `herdr status` reports "server: not running" and `herdr worktree list` errors `server_not_running`. Those answers are about the dead product, but they read as answers about the live one — that name collision already caused one real failure (the 2026-11-15 08:15 cron run concluded Herdr was down and routed around a healthy system). Since 0189/0190, all routing goes through `hc`; the handoff-mailbox skill records "drive via `hc`, never `herdr`."

Stephen's question, 2026-11-16: it's a cleanup issue — why is a dead binary still sitting there? Is there a reason? Nobody chose to keep it; it is migration residue from the Nov 13 switch. This task establishes whether anything still depends on it, and retires it if nothing does.

## Task
1. **Inventory (read-only).** Establish and quote:
   - How the 0.9.5 client is installed: binary path, file type (standalone binary / symlink / script), owning package if any (rpm/dnf, pip, cargo, manual install), and whether `hc` / herdr-core comes from the same install or a separate one.
   - Everything that invokes the `herdr` binary by name: your cron jobs and worker prompts, your skills (including handoff-mailbox), shell rc files and aliases, Herdr templates, any autostart/session files. Grep output quoted, with each hit classified: functional dependency, or informational mention (docs/notes saying "never use herdr" are mentions, not dependencies).
   - Whether Stephen's own access paths touch it: his phone reaches herdr-core over Tailscale (established) — state whether anything in the client binary's config (`~/.config/herdr/`) is shared with herdr-core's state, or fully separate.
2. **Retire — only if the inventory clears it.** If (a) no functional dependency invokes the `herdr` binary, and (b) it is separable from herdr-core's install, and (c) its config/state is not shared with herdr-core: **rename** the binary in place to `herdr-legacy-0.9.5` (same directory). Do not delete anything. Then verify and quote:
   - `hc server status` — same core id, same pid, same live terminal count as before the rename.
   - `hc ls` — the same panes (copilot-main, agy-main, shells), unchanged.
   - `herdr status` now fails as command-not-found (the loud failure is the point — a stale reference should break visibly, not answer wrongly).
   - Note plainly in the reply that the next mailbox worker cycle is the end-to-end check that nothing in the worker path needed the binary; do not simulate a cycle.
3. **Stop gate.** If any functional dependency exists, or the binary cannot be separated from herdr-core's install, or config/state is shared: change **nothing**, and report exactly what depends on it and why. That report is a complete, successful outcome for this task.
4. **Report-only appendix.** While you are in the same bin directories, list any other plainly dead binaries you notice (no current product, no references found) — names and paths only. Do not rename, move, or delete any of them; Stephen decides on the list.

## Success criteria (falsifiable)
- The reply quotes the install facts (path, type, package, relation to herdr-core's install) and the dependency grep with each hit classified.
- Either: the rename happened and the before/after `hc server status` + `hc ls` outputs match (same pid, same terminals), with `herdr` now command-not-found — or nothing changed and the blocking dependency is named exactly.
- herdr-core was never stopped or restarted in this task (same pid and uptime continuity in the before/after outputs prove it).

## Constraints
- **Rename only — delete nothing.** Every change in this task must be reversible by renaming back.
- Do not stop, restart, or reconfigure herdr-core; do not disturb any live pane.
- Do not change any cron job, worker prompt, skill, or config in this task — if a functional reference to `herdr` exists in one, that is a stop-gate report, not an invitation to edit it.
- Do not touch the Hermes harness, the mailbox, or either git repo in this task.
- If the installed layout differs from what Context describes, follow the machine's evidence and say so.
- **Cost:** one short run on the standard tier (DeepSeek Flash). Inventory and one rename — no model escalation, no Newton session.

## Proof
Host-local task: no branch, no CI — `branch: none` / `run: none` is the honest state. The staged reply carries the raw outputs (inventory, grep, before/after `hc` state) as the record; Juno verifies them against the facts quoted in Context.

## Reply format
One staged reply with these sections:
- Gate outcome at the top: retired (renamed) or stopped (dependency named)
- Install facts: path, type, package, relation to herdr-core
- Dependency inventory: grep hits, each classified functional / mention
- Rename + verification (if done): before/after `hc server status` and `hc ls`, `herdr` command-not-found output
- Appendix: other plainly dead binaries noticed (report only)
- Cost block
