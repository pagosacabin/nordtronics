---
task_id: "0279"
status: in_progress
iteration: 2
proof: []
notes: ""
expect-reply-within: 24h
---

# 0279 — Pocket Conquest /games game-state API — fix Juno mode (follow-up to 0275)

## Context

The Pocket Conquest page is live at https://nordtronics.io/pocket-conquest/ (deployed from
artifact pocket-conquest). Its Juno mode (async opponent) calls a game-state API on the base
URL: POST `{base}/games` (create a game), POST `{base}/games/{id}/move` (submit a turn),
GET `{base}/games/{id}` (poll the game JSON until Juno's reply lands). The base URL is
https://home.tail2bff3f.ts.net/pocket-conquest, published by Tailscale Serve from the 0275
bridge on 127.0.0.1:8767. Stephen already ran the Serve command, so the path is live.

0275 built the bridge (branch hermes/0275-pocket-conquest @ 08efcd2,
cabin-play:pocket-conquest-bridge/, deployed to
/home/astroboy/.local/share/pocket-conquest-bridge/app/, user unit
pocket-conquest-bridge.service enabled) and proved the MOVE endpoint:
POST /pocket-conquest-move -> {"move": "M B5 B4 1"} in 0.60 s, GET /health -> {"ok": true}.
But 0275 did NOT implement the /games game-state API the page calls. That is the gap this
task closes, and it is why the page fails in the browser: Juno mode reports "Couldn't reach
the Juno game API (Failed to fetch)" and falls back to the computer.

Juno verified from outside that the Serve path and the bridge are up: GET the base URL
returns the bridge's own 404 {"error": "not found"} (exact-path allowlist rejecting an
unknown path). But POST/OPTIONS to {base}/games hang/time out — the route does not exist.
The 0275 proof tested the move endpoint only; the /games contract was never validated
end to end. This task is the validation that was owed.

## Task

1. Route the build to Newton via Herdr (pure software; bench firmware stays with Hermes).
   Read the LIVE page source (https://nordtronics.io/pocket-conquest/index.html) and extract
   the EXACT Juno-mode API contract: the request/response shapes for POST /games,
   POST /games/{id}/move, and GET /games/{id} — field names, what a created game returns
   (game id and initial state), what a move submission takes, what the polled game JSON
   contains, and how the page detects Juno's reply and game end. Do not guess these shapes;
   the page source is the contract. Record the contract as found in the reply.
2. Read the 0275 bridge code (branch hermes/0275-pocket-conquest @ 08efcd2,
   cabin-play:pocket-conquest-bridge/server.py, pocket_conquest.py, agent_core.py) and the
   deployed copy at /home/astroboy/.local/share/pocket-conquest-bridge/app/. Work on a NEW
   branch hermes/0279-pocket-conquest-games-api; do not rewrite 0275's branch.
3. Implement the /games game-state API in the bridge to match the page's contract exactly,
   reusing the existing rules engine (legal-move generation, validation, combat, income)
   and the existing DeepSeek move logic (agent_core.py pick_legal). Game state must persist
   on the laptop (an in-memory dict keyed by game id is fine — it only needs to live as
   long as the service runs) so a created game can be polled after the player's move.
   POST /games creates a game and returns its id + initial state. POST /games/{id}/move
   accepts the player's turn, applies it through the engine, generates Juno's (DeepSeek)
   reply turn, and persists the updated game. GET /games/{id} returns the current game
   JSON so the page's poll sees Juno's reply.
4. CORS: the bridge must answer OPTIONS preflights and send Access-Control-Allow-Origin
   for https://nordtronics.io (and the exact headers the page sends, including
   content-type), so the browser at nordtronics.io can fetch the Tailscale URL. 0275's
   server.py already claims CORS — verify it actually covers the /games routes and the
   preflight; fix it if it does not.
5. Keep the existing endpoints working: POST /pocket-conquest-move must still return a
   legal move, GET /health must still return {"ok": true}. The /games API is additive —
   do not break what 0275 proved.
6. Make sure the service is running persistently: the user unit
   pocket-conquest-bridge.service is enabled — verify it is actually active and stays up,
   restart it on the new code, and confirm the deployed copy matches the new branch tip.
   Say plainly whether it survives a laptop reboot or needs a manual restart — do not claim
   reboot survival you have not proven.
7. Copilot peer review, locked lane (this is a backend API parsing network input): the
   reviewer answers the 7-item security checklist (no hardcoded secrets, inputs
   bounds-checked, auth on every endpoint, TLS off-box, least privilege, fail-closed
   errors, pinned deps), pass/fail each. Fix any fail before staging. Copilot's monthly
   quota is exhausted until Nov 30 at 5 PM — use Antigravity (Claude Sonnet 4.6) as the
   substitute reviewer (as 0275 did), and record that a Copilot locked-lane pass over the
   new commits is owed when the quota resets Nov 30.
8. Commit the code to cabin-play (play project — never nordtronics). Push via SSH; gh.py
   cannot write to cabin-play.
9. Prove it end to end, through the Tailscale URL (not just localhost): the full Juno-mode
   flow the page makes — POST /games (create), POST /games/{id}/move (a legal player
   turn), GET /games/{id} (poll, showing Juno's DeepSeek reply move, legal per the
   engine) — plus an OPTIONS preflight showing the CORS headers, plus /health and
   /pocket-conquest-move still working. Use the exact requests the page makes.

## Success criteria

- The bridge implements the /games game-state API matching the page's contract as read
  from the live page source (contract recorded in the reply).
- Full flow works through https://home.tail2bff3f.ts.net/pocket-conquest: POST /games
  returns a game id; POST /games/{id}/move with a legal player turn is accepted; GET
  /games/{id} returns game JSON showing Juno's DeepSeek reply move, legal per the engine,
  inside the page's timeout.
- CORS works: an OPTIONS preflight to /games from Origin https://nordtronics.io returns
  204 (or 200) with Access-Control-Allow-Origin: https://nordtronics.io, and a POST from
  that Origin succeeds (no browser block).
- Existing endpoints still work: POST /pocket-conquest-move returns a legal move;
  GET /health returns {"ok": true}.
- The page's "Start Juno game" works from the hosted page (nordtronics.io/pocket-conquest/)
  without "Failed to fetch" — proven by driving the exact requests the page makes.
- Service persistent: the user unit is active, the deployed copy matches the new branch
  tip, and reboot survival is stated honestly.
- Locked-lane review done (Antigravity substitute SHIP, or findings fixed; Copilot pass
  owed Nov 30 recorded).
- Code committed to cabin-play on the new branch.

## Constraints

- Cost: DeepSeek flash, thinking disabled, low temp, small max tokens (same as 0275).
  Report per-move token counts in the reply. No other model, no paid tier.
- The DeepSeek key never leaves `~/.hermes/` and never appears in code, the task file, or
  the staged reply.
- Do not change the page (nordtronics.io/pocket-conquest/ or the artifact). Do not change
  the Tailscale Serve path (/pocket-conquest) or the port (8767). Do not change the rules
  of Pocket Conquest.
- Never break the page: on any failure the endpoint errors out and the built-in computer
  takes over.
- Do not reboot the laptop — 0262/0264 own the reboot.
- The Serve path carries no OAuth (the page cannot log in). Tailnet only is fine —
  Stephen's phone is on the tailnet.

## Proof

- The /games contract as read from the live page source.
- Curl transcript through the Tailscale URL: POST /games, POST /games/{id}/move,
  GET /games/{id} (showing Juno's DeepSeek reply), OPTIONS preflight (CORS headers),
  plus /health and /pocket-conquest-move still working.
- Cabin-play branch/commit + locked-lane verdict (Antigravity substitute; Copilot pass
  owed Nov 30 recorded).
- Service status (systemctl --user status pocket-conquest-bridge) showing active, and
  the deployed copy matching the branch tip.
- Per-move token counts.

## Reply format

Short report: the /games contract as found, the curl proof (full flow + CORS + existing
endpoints), the review verdict, the service status, per-move token counts, and the
reboot-survival statement.

## Assignment

Hermes routes the build to Newton via Herdr; Newton builds the API. Hermes owns the
service/bench verification on the laptop (Serve is already published; the bridge must be
running and answering) and stages the reply. Copilot peer-reviews when quota returns;
Antigravity substitutes until then. Juno verifies independently.
