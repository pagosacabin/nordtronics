---
task_id: "0283"
status: filed
iteration: 1
proof: []
notes: "Filed 2026-11-27 on Stephen's word ('per new game we should have random start places and random towns'). Prerequisite for the page randomization: the bridge must accept a variable board before the page starts sending one, or Juno mode breaks. Juno randomizes the page only after this task is deployed."
expect-reply-within: 24h
---

# 0283 — Pocket Conquest bridge: accept a variable board layout (random capitals/towns)

## Context

Stephen wants every new Pocket Conquest game to randomize the starting places
(capitals) and the neutral towns, instead of the fixed B5/E1 capitals and D1/B3/E5
towns. Juno will randomize the page (nordtronics.io/pocket-conquest/) once the
bridge can accept a board that is not the fixed one. Right now it cannot: the
bridge hardcodes the layout, and a randomized board would break Juno mode.

What is hardcoded, and where:
- `pocket-conquest-bridge/game_state.py`: `CAPITAL_SIDE = {"B5": "S", "E1": "R"}`.
  `engine_position_from_page` sets `capital = CAPITAL_SIDE[name]` for every
  capital tile. If the page sends a capital at any other tile, that lookup raises
  KeyError and the move fails. `DEFAULT_TOWN_NAMES` is only a fallback — the page
  already sends the town `name`, so towns at new tiles are fine as long as a name
  arrives.
- The rules engine itself (`pocket_conquest.py`) does NOT hardcode the layout:
  `normalise_position` / `_normalise_tile` derive capitals and towns from each
  tile's `terrain` field (a capital tile must carry `capital` "S" or "R"; a town
  tile must carry a town dict with name and militia). No engine change is needed.

The one thing the engine needs that the page payload does not carry is each
capital's ORIGINAL side: the engine's `capital` field is the tile's original
side (it is how `winner()` detects a captured capital). Once a capital is
captured its owner changes, so the original side cannot be recovered from the
current owner. The bridge must remember it.

## Task

1. Route the build to Newton via Herdr (pure software; bench firmware stays with
   Hermes).
2. In `game_state.py` (branch hermes/0279-pocket-conquest-games-api is the base;
   work on a NEW branch hermes/0283-variable-board), remove the hardcoded
   `CAPITAL_SIDE` lookup as the source of truth. Instead, track each game's
   capital tiles and their original sides **at game creation** (POST /games):
   when a game is created, record which tiles are capitals and which side each
   belongs to (from the posted board's capital tiles and their owners at
   creation — at creation a capital's owner IS its original side). Store that
   per-game mapping in the GameStore entry alongside the position. On later
   moves (POST /games/{id}/move and the poll), use the stored per-game mapping
   for the `capital` field instead of the hardcoded dict. If a game id was
   adopted (not created by this service) and no mapping was stored, fall back
   to the fixed B5/E1 mapping so old games keep working.
3. Keep the fixed board working: a game created with capitals at B5/E1 and
   towns at D1/B3/E5 must behave exactly as before (same proof as 0279).
4. Towns: confirm a town at any tile works as long as the payload carries its
   `name` (the page sends it). Do not hardcode town locations.
5. Work on a new branch hermes/0283-variable-board. Commit to cabin-play
   (play project — never nordtronics). Push via SSH; gh.py cannot write to
   cabin-play. Deploy to the laptop service (pocket-conquest-bridge.service)
   and restart it on the new code, confirming the deployed copy matches the
   new branch tip.
6. Copilot peer review, locked lane (this is a backend API parsing network
   input): the reviewer answers the 7-item security checklist (no hardcoded
   secrets, inputs bounds-checked, auth on every endpoint, TLS off-box, least
   privilege, fail-closed errors, pinned deps), pass/fail each. Fix any fail
   before staging. Copilot's monthly quota is exhausted until Nov 30 at 5 PM —
   use Antigravity as the substitute reviewer (as 0275/0279 did; note which
   model answered, since pools have been exhausting), and record that a Copilot
   locked-lane pass over the new commits is owed when the quota resets Nov 30.
7. Prove it end to end, through the Tailscale URL (not just localhost), with a
   RANDOMIZED board: capitals NOT at B5/E1 and towns NOT at D1/B3/E5. Show
   POST /games (create with the randomized board), POST /games/{id}/move (a
   legal player turn), GET /games/{id} (poll, showing Juno's DeepSeek reply
   move, legal per the engine), plus /health and /pocket-conquest-move still
   working. Also prove a captured capital is still detected with the per-game
   mapping: drive a position where Juno's side takes the randomized capital
   and confirm the game reports the winner (or drive it through the engine
   directly if a full game is too long — state which you did). Use the exact
   requests the page makes.

## Success criteria

- A game created through https://home.tail2bff3f.ts.net/pocket-conquest with
  capitals NOT at B5/E1 and towns NOT at D1/B3/E5 works end to end: create,
  move, poll, Juno's reply legal, inside the page's timeout.
- A game created with the fixed board (B5/E1, D1/B3/E5) still works exactly as
  before (0279 proof re-run or referenced).
- A captured capital is detected correctly with the per-game mapping (winner
  reported).
- The existing endpoints still work: GET /health returns {"ok": true};
  POST /pocket-conquest-move returns a legal move.
- Service persistent: the user unit is active, the deployed copy matches the
  new branch tip, and reboot survival is stated honestly.
- Locked-lane review done (substitute SHIP, or findings fixed; Copilot pass
  owed Nov 30 recorded).
- Code committed to cabin-play on the new branch.

## Constraints

- Cost: DeepSeek flash, thinking disabled, low temp, small max tokens (same as
  0275/0279). Report per-move token counts in the reply. No other model, no
  paid tier.
- The DeepSeek key never leaves `~/.hermes/` and never appears in code, the
  task file, or the staged reply.
- Do not change the page (nordtronics.io/pocket-conquest/ or the artifact).
  Do not change the Tailscale Serve path (/pocket-conquest) or the port (8767).
  Do not change the rules of Pocket Conquest.
- Never break the page: on any failure the endpoint errors out and the
  built-in computer takes over.
- Do not reboot the laptop — 0262/0264 own the reboot.
- The Serve path carries no OAuth (the page cannot log in). Tailnet only is
  fine — Stephen's phone is on the tailnet.

## Proof

- Curl transcript through the Tailscale URL with a RANDOMIZED board: POST
  /games, POST /games/{id}/move, GET /games/{id} (showing Juno's DeepSeek
  reply), plus the fixed-board re-check, the captured-capital check, /health
  and /pocket-conquest-move still working.
- Cabin-play branch/commit + locked-lane verdict (substitute; Copilot pass
  owed Nov 30 recorded).
- Service status (systemctl --user status pocket-conquest-bridge) showing
  active, and the deployed copy matching the branch tip.
- Per-move token counts.

## Reply format

Short report: the per-game capital mapping design, the curl proof
(randomized board + fixed board + captured capital + existing endpoints), the
review verdict, the service status, per-move token counts, and the
reboot-survival statement.

## Assignment

Hermes routes the build to Newton via Herdr; Newton builds. Hermes owns the
service verification on the laptop (Serve is already published; the bridge must
be running and answering) and stages the reply. Copilot peer-reviews when quota
returns; a substitute reviews until then. Juno verifies independently, and
randomizes the page only after this task is deployed.
