---
task_id: "0270"
title: "Chess MCP server on the laptop (Hermes plays through tools)"
status: staged
iteration: 2
expect-reply-within: 6h
proof:
  branch: hermes/0270-chess-mcp
  sha: 31a928f9f0c66389cad479344e99731b593ca7e1
  run: none
  copilot_review: "SHIP at 31a928f (locked lane, three passes: FIX -> FIX -> SHIP)"
  files:
    - chess-mcp/server.py
    - chess-mcp/tests/mcp_session_proof.py
    - chess-mcp/requirements.txt
    - chess-mcp/README.md
    - chess-mcp/.gitignore
  evidence:
    - mailbox/files/0270/transcript.txt
    - mailbox/files/0270/page-check.txt
    - mailbox/files/0270/page-shows-move.png
    - mailbox/files/0270/copilot-review-1-quoted.txt
    - mailbox/files/0270/copilot-review-2.txt
    - mailbox/files/0270/copilot-review-3.txt
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
  rates_used: unknown
  cost_usd: unknown
notes: |
  Delivered: a stdio MCP server in cabin-play with get_game / list_legal_moves /
  make_move, proven end to end over the real MCP client, reviewed by Copilot in the
  locked lane (three passes: FIX -> FIX -> SHIP).

  BUILD ROUTING — DEVIATION, declared. Newton was routed the build in Herdr pane t19
  (agy-main, Gemini 3.8 Flash high; both pools read before the run: Gemini 89%,
  Claude/GPT 100%). Over ~12 minutes and five turns it explored the environment and
  spent two approval prompts on `python -c` probes without writing a single file, so
  the turn was interrupted (`hc interrupt t19` -> esc) and Hermes authored the server
  directly. No output of Newton's was used; nothing of his is in the commit. Same
  pattern as 0268's Newton attempt — the routing is working, the agent is not
  producing files on this kind of task.

  WHAT SHIPPED (cabin-play, branch hermes/0270-chess-mcp, tip 31a928f):
   chess-mcp/server.py            FastMCP/MCPServer over stdio; three tools; urllib
                                  HTTP; validation and fail-closed errors
   chess-mcp/tests/mcp_session_proof.py  real MCP client harness (see below)
   chess-mcp/requirements.txt     chess==1.11.2, mcp==2.3.0 (exact pins)
   chess-mcp/README.md            start command, client config, the fen traps, security
   chess-mcp/.gitignore

  TWO FACTS THE BUILD TURNED UP, both found by reading the live page rather than guessing:
   1. The MCP Python SDK renamed its high-level server: mcp 2.x has no
      mcp.server.fastmcp; it is `from mcp.server.mcpserver import MCPServer`. server.py
      imports MCPServer and falls back to FastMCP so it runs on 1.x and 2.x. (Newton's
      first failed probe was exactly this import.)
   2. The game JSON's `fen` is client-written, and the page posts the position AFTER the
      move. Worse, the page's own toFEN writes the en-passant target unconditionally
      ("... KQkq a6"), while python-chess omits it unless an ep capture is legal. A
      server posting plain board.fen() therefore stores a fen the page cannot match, and
      the page's resume path silently falls back to a bare position with an EMPTY move
      list — a move lands on the server and the page still looks wrong. server.py now
      posts board.fen(en_passant="fen"), the page's dialect. The negative control below
      is exactly that failure.

  PROOF (all commands run this tick; artifacts in mailbox/files/0270/):
   - transcript.txt — tests/mcp_session_proof.py at 31a928f: real MCP client spawns the
     server over stdio, initialize -> list_tools (get_game, list_legal_moves, make_move),
     get_game on the fresh game mcp8tncv, list_legal_moves (20 legal), REFUSALS: illegal
     e2e5 -> "e2e5 is not legal in this position", wrong turn (side=black, move 1) ->
     "not black's turn (white to move)", non-string side (0) -> SDK arity rejection;
     get_game confirms the move count did NOT advance (0) after the three refusals; then
     a2a3 and a7a5 played through make_move, each verified by the server's own post-POST
     re-read; GET /games/mcp8tncv.json shows moves ["a3","a5"].
   - page-check.txt + page-shows-move.png — headless Chromium with a fresh profile loads
     https://nordtronics.io/chess/, seeds the game code through the page's own localStorage
     resume path, and reports: label mcp8tncv, move list "1. a3  a5", two last-move
     squares highlighted, status "Your move — White" => PASS. NEGATIVE CONTROL, same
     harness, game api-probe-1 (whose fen was posted before the move): move list EMPTY and
     no highlight — the page silently degrades, which is why the dialect matters.
   - copilot-review-1-quoted.txt / -2.txt / -3.txt — the three locked-lane passes.
     Pass 1 (48c8637) Verdict: FIX — HIGH non-string side raised AttributeError; MED
     non-string fen raised TypeError; MED /api/move never verified; LOW proof left a
     public game. Pass 2 (467328b) all four resolved, FIX again on: MED falsey side
     bypass, LOW `--game` without an id silently created a game, LOW stale README proof
     section. Pass 3 (31a928f) all resolved, no new defects, Verdict: SHIP.
   - The 7-item security checklist at SHIP: 1 no hardcoded secrets PASS; 2 inputs
     bounds-checked PASS; 3 auth on every endpoint — FAIL BY DESIGN (the public game API
     has no authentication and this task forbids adding one; the boundary is stdio, no
     port); 4 TLS PASS (base must be https, plain http only behind
     CHESS_API_ALLOW_INSECURE=1); 5 least privilege PASS; 6 fail-closed PASS (typed
     errors, move re-read); 7 pinned deps PASS.

  NOT DONE, deliberately: the server is not wired into Hermes's own mcp_servers config.
  That is a config.yaml write plus a new session to load the tools, and the task did not
  ask for it; README carries the exact snippet. `run: none` below is earned: cabin-play
  has no .github/workflows at all (`ls .github` -> not found; `git ls-tree -r --name-only
  origin/main | grep -c '^.github/workflows'` -> 0), so no run exists to cite, and no
  ntfy receipt applies (no compiled artifact).

  WHICH GAME: main carries 0 files under chess-mcp/ (the deliverable lives only on the
  branch), so diff against the branch tip, not main.

  Test games left on the public API — the game API has NO delete endpoint (DELETE and
  POST /chess/games/<id>.json both answer 405), so every probe left one behind:
  recon-0000, api-probe-1 (the negative control), mcp-proof-1795795158, mcpmg3jv,
  mcpg3nih, mcpamm79, mcpcdx6n, mcp8tncv (the proof game). They are throwaway ids in the
  games directory on the VPS; remove them server-side. Not hidden, not claimed as clean.

  cost: unknown — this harness surfaced no per-call usage objects for the tick, so no
  token or dollar figure is estimated. One DeepSeek model turn is not what this task
  spent; the expensive part was three Copilot review passes in the pane
  (Session counter 124.74 -> 127.29 AIC) and Newton's aborted Gemini turn.
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

## Reply — 0270

**Start it** (stdio; the MCP client spawns it):

```bash
/home/astroboy/.local/share/chess-mcp/venv/bin/python /home/astroboy/cabin-play/chess-mcp/server.py
```

Hermes config (`mcp_servers:`), tools load on the next session:

```yaml
mcp_servers:
  chess:
    command: /home/astroboy/.local/share/chess-mcp/venv/bin/python
    args: ["/home/astroboy/cabin-play/chess-mcp/server.py"]
```

**Tools:** `get_game(game_id)`, `list_legal_moves(game_id | fen)`,
`make_move(game_id, move_uci[, side])` — UCI in, SAN + new FEN out, refusals for
illegal moves, the wrong side's turn and finished games.

**Proof move:** game `mcp8tncv`, played `a3` then `a5` through `make_move` over a real
MCP stdio session; `GET https://nordtronics.io/chess/games/mcp8tncv.json` returns
`{"moves":["a3","a5"], "fen":"… w KQkq a6 0 2", "status":"active"}`, and the page shows
`1. a3  a5` with both squares highlighted (`page-shows-move.png`).

**Copilot verdict:** SHIP at 31a928f after three locked-lane passes (first pass FIX with
1 HIGH + 2 MED + 1 LOW, all fixed and re-verified).

**Two things worth knowing before 0271:** the game JSON's `fen` must be the *post-move*
fen in the *page's* dialect (`en_passant="fen"`) or the page silently drops the move
list; and the MCP SDK 2.x import is `mcp.server.mcpserver.MCPServer`. Both are in the
README.
