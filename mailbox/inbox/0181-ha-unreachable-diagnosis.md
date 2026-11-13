---
task_id: '0181'
protocol_version: 1.2.0
status: inbox
iteration: 0
expect-reply-within: 4h
---

# 0181 — Diagnose: Home Assistant unreachable from Stephen's phone (Tailscale IP timeout)

## Assignment

- Hermes: all steps — a read-only diagnosis from the laptop, the only machine
  with both LAN and tailnet vantage. Newton is not needed.

## Context

- Stephen's HA companion app worked last night (Nov 12, before bed — he turned
  lights off with it) and times out this morning (Nov 13 ~09:47 MST):
  "Unable to connect to Home Assistant. Connection timed out." The app is
  connecting to http://100.76.58.105:8123 — the HA Pi's tailnet IP, set as the
  app's external URL on Nov 12.
- Tailnet machines: home (the laptop), stephens-a35 (phone), homeassistant
  (HAOS on Raspberry Pi, 100.76.58.105).
- Stephen is out at an appointment and asked Juno/Hermes to look. Juno is not
  on the tailnet and cannot probe the Pi herself.
- Same morning, probably unrelated: Stephen installed Docker + n8n on the
  laptop and built a workflow with a Home Assistant node around 09:34. Whether
  the laptop can reach HA *right now* also bears on whether that workflow
  could run — report it either way.

## Task

Read-only diagnosis, in this order:

1. `tailscale status` — report the `homeassistant` peer line (online/offline,
   last seen) and the laptop's own state.
2. From the laptop: `curl -sS -o /dev/null -w '%{http_code}' --max-time 10
   http://100.76.58.105:8123/` — does HA answer over the tailnet?
3. From the laptop on the LAN: ping `homeassistant.local` (or the Pi's LAN IP
   if you know it) and curl port 8123 there — does HA answer on the LAN?
4. Read the three results together and name the broken layer:
   LAN yes / tailnet no → the Pi's Tailscale add-on is the suspect.
   Neither → the Pi or HA itself looks down.
   Both fine from the laptop → the fault is phone-side (the phone's Tailscale
   app/VPN), say so plainly.

## Success criteria

- A staged reply naming the broken layer (or stating it could not be
  determined, and why), with the raw output of steps 1–3 pasted in the body.

## Constraints

- Read-only: no restarts, no config changes, no add-on reinstalls, no fixes.
  Diagnosis only — Stephen decides next steps when he is back.
- Do not touch the n8n/Docker setup.
- Cost: flash tier; this is a handful of commands, keep it small.

## Proof

Host-only read-only task: no branch, no CI. Put `proof: []` in the staged
front matter with a note saying so; the pasted terminal output in the reply
body is the evidence.

## Reply format

Stage the reply under `mailbox/staged/` with protocol 1.2.0 front matter
(status, cost block, proof as above). Body: the verdict first (which layer is
broken), then the step 1–3 outputs.
