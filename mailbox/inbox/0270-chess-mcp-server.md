---
task_id: "0270"
status: filed
iteration: 1
proof: []
notes: ""
expect-reply-within: 6h
---

# 0270 — Chess MCP server on the laptop (Hermes plays through tools)

## Context

Stephen's chess page (https://nordtronics.io/chess/) has three opponent
modes; the third was always "a local agent with its own logic over MCP".
The page speaks plain HTTP (the agent-endpoint box), but MCP agents
cannot use that — they need an MCP server exposing chess as tools.
Hermes already has MCP tooling (KiCad MCP plus others, proven on his
machine). This task builds the server so Hermes can play through MCP.
The page does not change: it keeps polling the game JSON, and the MCP
server talks to the same game store the page uses.

The game API (live on the VPS, no auth — same API the page uses):
- `GET https://nordtronics.io/chess/games/<id>.json` — the game file
  (`{"id","mode","moves":[SAN…],"fen","juno_side","status","created","updated"}`).
- `POST https://nordtronics.io/chess/api/new {"game_id","juno_side"}` —
  create a game (test games: fresh random id).
- `POST https://nordtronics.io/chess/api/move {"game_id","move" (SAN),"fen","side"}` —
  submit a move; 409 if the game is finished or it is not that side's
  turn. Turn = move-count parity (even = White to move, odd = Black).

## Task

1. Route the build to Newton via Herdr (pure software; bench firmware
   stays with Hermes). Newton builds a small MCP server in Python using
   the official MCP Python SDK (`mcp` package, FastMCP server). If the
   SDK cannot be installed on the laptop, STOP and stage that finding
   instead — do not fake it.
2. Transport: stdio (the MCP client spawns the server on the laptop).
   No network port, no public endpoint in this task — a remote HTTP
   transport is a follow-up, not this task.
3. Tools (names are Newton's pick, behaviour is fixed):
   - get_game(game_id): return the game's FEN, move list (SAN), side to
     move, and status.
   - list_legal_moves(game_id or fen): return the legal moves (UCI).
   - make_move(game_id, move): take a UCI move, validate it is legal
     for the current position (python-chess or equivalent — name it),
     compute the SAN and the new FEN, and submit it with POST
     /chess/api/move. Return the new FEN and the move played. Refuse
     (do not submit) an illegal move, a move on the wrong side's turn,
     or a move in a finished game.
4. Commit the code to cabin-play (play project — never nordtronics).
   Push via SSH; gh.py cannot write to cabin-play.
5. Copilot peer review, locked lane (this service writes game state
   over the network and parses client input): the reviewer answers the
   7-item security checklist (no hardcoded secrets, inputs
   bounds-checked, auth on every endpoint, TLS off-box, least
   privilege, fail-closed errors, pinned deps), pass/fail each. Fix any
   fail before staging.
6. Prove it end to end: Hermes connects his MCP client to the server
   (stdio), creates a test game id, reads the position through
   get_game, lists legal moves, and submits one legal move through
   make_move. Show the move landed in the public game JSON
   (GET the game file) and the page shows it. Delete the test game
   afterwards (or leave the id in the reply so Juno can remove it).

## Success criteria

- MCP server runs on the laptop over stdio; Hermes's client connects.
- The three tools behave as specified, including refusing an illegal
  move and a wrong-turn move.
- A test-game move made through the tools lands in the public game
  JSON and shows on the page.
- Copilot locked-lane review done (SHIP, or findings fixed).
- Code committed to cabin-play.

## Constraints

- No secrets in code, the task file, or the staged reply. The server
  needs no API key — the game API carries none.
- Do not change the chess page or the VPS game API in this task.
- Do not expose a public endpoint — stdio only.
- Do not reboot the laptop — 0262/0264 own the reboot.
- This task does NOT solve turn notification: nothing here tells
  Hermes it is his turn in a live game. That is a follow-up once the
  server works.

## Proof

- Cabin-play branch/commit + Copilot locked-lane verdict.
- Transcript (or log) of the MCP session: get_game, list_legal_moves,
  make_move, plus the game JSON showing the move landed.

## Reply format

Short report: how to start the server, the tool names, the proof move
(game id + move), Copilot verdict.

## Assignment

Hermes routes the build to Newton via Herdr; Newton builds. Copilot
peer-reviews (locked lane, GitHub-bound — code goes to cabin-play).
Juno verifies independently.
