---
task_id: "0136"
protocol_version: 1.0.0
status: in_progress
expect-reply-within: 6h
iteration: 1
---

# 0136 — Relocate Guition firmware mirror to cabin-play

## context

Task 0134 fixed the JC4827W543 battery ADC (scale factor x1.32998 -> x1.72727, verified
and archived 2026-10-15). Because the JC4827W543-ESPHome repo 404'd, the fixed firmware
was mirrored into the nordtronics repo on branch `hermes/0134-guition-battery-adc` under
`firmware/guition-jc4827w543/` (three files: `jc4827w543.yaml`, `README.md`,
`0134-battery-log.txt`; the yaml is byte-identical to the flashed build, sha256
`d355ded833659ec972442612df107e42b8f978e49438c037226e607083d965cc`).

Stephen ruled 2026-10-15: play projects live in `pagosacabin/cabin-play` (renamed from
Pi-projects that morning), NEVER in nordtronics. The mirror must move.

## task

Copy the Guition firmware tree out of nordtronics and into `pagosacabin/cabin-play`:

1. Read the three files from nordtronics branch `hermes/0134-guition-battery-adc`,
   path `firmware/guition-jc4827w543/`.
2. Commit them to `pagosacabin/cabin-play` on main under `guition-jc4827w543/`
   (same filenames). Verify the yaml's sha256 matches
   `d355ded833659ec972442612df107e42b8f978e49438c037226e607083d965cc` after the copy.
3. Update the cabin-play README for that directory (or the repo README if there is no
   directory readme) with one line stating provenance: mirrored from 0134, verified
   2026-10-15, scale-factor fix x1.72727.
4. Do NOT touch nordtronics main. Do NOT create the JC4827W543-ESPHome repo.

## success_criteria

- `guition-jc4827w543/jc4827w543.yaml`, `README.md`, `0134-battery-log.txt` exist on
  `pagosacabin/cabin-play` main, yaml sha256 matches the known value.
- Commit SHA on cabin-play cited.

## constraints

- One deliverable: the relocated tree. No UI work, no firmware changes.
- No secrets in the task file or staged reply.

## proof

- Commit SHA on `pagosacabin/cabin-play` main.
- `sha256sum` of the copied yaml.

## reply_format

Stage the reply under `mailbox/staged/0136-guition-relocate-to-cabin-play.md` with
front matter (`task_id: 0136`, `status: staged`, `iteration: 1`), the proof pointers
above, and a `notes:` field describing anything unexpected.
