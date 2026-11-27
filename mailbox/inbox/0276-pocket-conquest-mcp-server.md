---
task_id: "0276"
status: filed
iteration: 1
proof: []
notes: ""
expect-reply-within: 24h
---

# 0276 — Pocket Conquest MCP server (agent plays via tools)

## Context

Pocket Conquest is a second game on the same pluggable-opponent platform as chess, built as a
separate page (Juno builds the page; chess is untouched). The chess MCP server (0270) gives
an agent three tools — get_game, list_legal_moves, make_move — backed by the VPS chess API.
This task is the Pocket Conquest version of that server, so an agent (Hermes) can play Pocket
Conquest through tools instead of through the browser.

Depends on 0275: build on the rules engine it delivers (same repo path in cabin-play). If
0275 has not staged yet, wait for it — do not rebuild the rules engine from scratch. If 0275
is blocked only on its Serve step, the engine is still committed and usable — proceed using
that engine and note it in the reply.

The game API: the Pocket Conquest game API on the VPS follows the chess API contract
(POST /pocket-conquest/api/new, POST /pocket-conquest/api/move, GET
/pocket-conquest/games/<id>.json; games stored as JSON). Juno owns standing up the VPS API.
Prove the MCP end-to-end against that documented contract; if the VPS API is not live yet,
prove against a local stand-in implementing the same contract and record that the live VPS
API is the remaining piece. Do not deploy to the VPS yourself — you have no VPS access.

### The game — full rules (same as 0275; do not invent or change these)

6x5 grid, columns A-F, rows 1-5, 30 tiles. Movement orthogonal only (no diagonals).
Nordhold (S) vs Ember Court (R). Each side starts with 3 armies at its capital and 10 gold.

Initial board:
```
     A     B     C     D     E     F
 1   .     H     .     T1    R3*   H
 2   .     .     H     .     .     .
 3   .     T2    .     .     .     .
 4   .     .     .     H     .     .
 5   H     S3*   .     .     T1    .
```
* = capital. H = hills (defender +1). T = neutral town, number is its militia.
Capitals: B5 = Nordhold (S), E1 = Emberhold (R). Neutral towns: D1 = Northford (militia 1),
B3 = Crossroads (militia 2), E5 = Southmere (militia 1). Hills: B1, C2, D4, F1, A5.

Turn (players alternate): (1) Build — 5 gold per army, only at your capital or a town you
own; newly built armies cannot move the turn they are built; a town captured this turn
cannot be built at until next turn. (2) Move / Attack — each stack moves 1 tile
(orthogonal); a stack cannot move twice in a turn; arrivals cannot move further or join an
attack that same turn; stacks may split per-army; moving into an empty tile claims it;
moving into an empty town takes it; moving into an enemy-occupied tile is an attack.
(3) Collect income at end of your turn: capital 5, each town you own 4, each plain tile
you own 2.

Combat: attacker total = attacking armies + 1d6; defender total = defending armies + 1d6 + 1
if the defender is on hills. Higher total wins; on a tie the defender holds. Loser loses 1
army, or 2 if the margin is 3 or more. Winner loses nothing. Losses capped at 2 per battle.
A tile falls only when its defenders are wiped out (reduced to 0); if defenders remain, the
attacker's stack stays adjacent. Militia never regenerates. Pincer: adjacent friendly stacks
may combine into one attack — one roll, armies summed; the main stack advances on a win,
the support stays put. Win: occupy the enemy capital.

## Task

1. Route the build to Newton via Herdr (pure software; bench firmware stays with Hermes).
   Build an MCP server on the laptop using the official MCP Python SDK (`mcp` package /
   FastMCP). If the SDK cannot be installed, STOP and stage that finding instead of
   building a hand-rolled protocol. **stdio transport** (no network port, no public
   endpoint). Reuse the 0275 rules engine for legality — do not rebuild it.
2. Three tools:
   - `get_game` — return the current position (board, gold, armies, side to move, status).
   - `list_legal_moves` — return the legal moves for the side to move (same move format
     as 0275 defines).
   - `make_move` — take a move, validate it is legal with the rules engine, compute the
     new state, and submit it via POST to the Pocket Conquest game API (contract above).
     Refuse illegal moves, wrong-turn moves, and moves on a finished game — return a clear
     error, never a silent fallback.
3. The tools must carry the full rules in their descriptions (the calling agent has never
   seen this game): each tool's description states the game is Pocket Conquest and points
   at the rules (a short rules summary in the description, plus the full rules in the
   repo README). A model calling these tools cold must be able to play legally from the
   tool descriptions plus the legal-moves list.
4. Copilot peer review, locked lane (this is a backend API parsing network input): the
   reviewer answers the 7-item security checklist (no hardcoded secrets, inputs
   bounds-checked, auth on every endpoint, TLS off-box, least privilege, fail-closed
   errors, pinned deps), pass/fail each. Fix any fail before staging.
5. Commit the code to cabin-play (play project — never nordtronics). Push via SSH; gh.py
   cannot write to cabin-play.
6. Prove it: Hermes connects his MCP client to the server, creates a test game, reads the
   position, lists the legal moves, and submits one legal move. The move lands in the game
   JSON (live API or local stand-in) and the state updates correctly. Delete the test game
   afterward, or leave its id in the reply for Juno to remove.

This task explicitly does NOT solve turn notification. The MCP server cannot tap Hermes on
the shoulder; the game (or Juno's watcher) files a "your move" task and his worker picks it
up. Do not build a notification mechanism in this task.

## Success criteria

- MCP server runs over stdio; the three tools work as specified.
- A legal move submitted through make_move lands in the game JSON and the state updates.
- Illegal / wrong-turn / finished-game moves are refused with a clear error.
- Copilot locked-lane review done (SHIP, or findings fixed).
- Code committed to cabin-play.
- The tool descriptions carry enough of the rules for a cold model to play legally.

## Constraints

- Cost: no model calls in this task's build itself beyond what Hermes/Newton already use;
  the server itself makes no model calls (it is tools + rules engine only).
- No secrets in code, the task file, or the staged reply. No API keys in the MCP server.
- The rules above are fixed — do not invent, change, or simplify them.
- Do not deploy to the VPS. Do not reboot the laptop — 0262/0264 own the reboot.
- This task does not solve turn notification; do not build one.

## Proof

- Cabin-play branch/commit + Copilot locked-lane verdict.
- Transcript of the MCP client session (get_game, list_legal_moves, make_move, and the
  resulting game JSON showing the move landed).
- Test game id (deleted, or left for Juno to remove).

## Reply format

Short report: how to start the server, the three tools with one-line descriptions, the
proof transcript, Copilot verdict, test game id and its disposal.

## Assignment

Hermes routes the build to Newton via Herdr; Newton builds. Copilot peer-reviews (locked
lane, GitHub-bound — code goes to cabin-play). Juno verifies independently.
