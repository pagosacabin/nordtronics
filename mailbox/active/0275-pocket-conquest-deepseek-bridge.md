---
task_id: "0275"
status: in_progress
iteration: 2
proof: []
notes: ""
expect-reply-within: 24h
---

# 0275 — Pocket Conquest rules engine + DeepSeek bridge (interim opponent)

## Context

Pocket Conquest is a second game on the same pluggable-opponent platform as chess, built as a
separate page (Juno builds the page; chess is untouched). Stephen and Juno played it once in
chat (Oct 20, 2026, 48 turns, Stephen's Nordhold beat Juno's Ember Court). It is a custom
game — DeepSeek and Hermes do NOT know it. Unlike chess, the opponent cannot rely on the model
already knowing the rules: the bridge prompt must TEACH the game every move (full rules +
current board + gold + legal moves + a bit of strategy). The legal-moves list keeps the reply
legal; the rules in the prompt are what make it play sensibly instead of picking at random.

The page (artifact pocket-conquest) has an "agent endpoint" box. The page calls the endpoint
directly from the browser: POST `{"position": {...}, "moves": [...]}` and expects a move back,
with a 30-second timeout, falling back to the built-in computer if the endpoint is empty,
unreachable, or illegal. The endpoint must be HTTPS (the page is HTTPS — plain http is blocked
as mixed content) and reachable from Stephen's phone browser. Stephen's phone is on the
tailnet, so a Tailscale Serve URL works.

### The game — full rules (do not invent or change these)

6x5 grid, columns A-F, rows 1-5, 30 tiles. Movement is orthogonal only (no diagonals).
Two factions: Nordhold (S, Stephen) vs Ember Court (R, rival).

Initial board:
```
     A     B     C     D     E     F
 1   .     H     .     T1    R3*   H
 2   .     .     H     .     .     .
 3   .     T2    .     .     .     .
 4   .     .     .     H     .     .
 5   H     S3*   .     .     T1    .
```
* = capital. H = hills (defender +1 in combat). T = neutral town, number is its militia.
Capitals: B5 = Nordhold (S, 3 armies), E1 = Emberhold (R, 3 armies).
Neutral towns: D1 = Northford (militia 1), B3 = Crossroads (militia 2), E5 = Southmere (militia 1).
Hills: B1, C2, D4, F1, A5. All other tiles are open plains.
Each side starts with 3 armies at its capital and 10 gold. Militia never regenerates.

Turn (players alternate). On your turn, in order:
1. **Build** — spend gold to raise armies at your capital or a town you own: 5 gold per army,
   as many as you can afford, no per-tile cap. Newly built armies cannot move the turn they
   are built. A town captured this turn cannot be built at until next turn.
2. **Move / Attack** — each stack moves 1 tile (orthogonal). A stack cannot move twice in a
   turn. A stack that arrives at a tile cannot move further or join an attack that same turn.
   Stacks may split per-army. Moving into an empty tile claims it; moving into an empty town
   takes it. Moving into an enemy-occupied tile is an attack.
3. **Collect income** — at the end of your turn: capital 5 gold, each town you own 4 gold,
   each plain tile you own 2 gold.

Combat (one battle = one attack on one adjacent tile):
- Attacker total = attacking armies + 1d6. Defender total = defending armies + 1d6 + 1 if the
  defender is on a hills tile. Higher total wins; on a tie the defender holds.
- Losses, capped at 2 per battle: the loser loses 1 army, or 2 if the margin is 3 or more.
  The winner loses nothing.
- A tile falls only when its defenders are wiped out (reduced to 0). If defenders remain,
  the attacker's stack stays adjacent and does not move in.
- Pincer (house rule): adjacent friendly stacks may combine into one attack — one roll,
  armies summed. The main stack advances on a win; the support stack stays put.

Win: occupy the enemy capital. Lose if your capital is taken.

## Task

1. Route the build to Newton via Herdr (pure software; bench firmware stays with Hermes).
   Newton builds a Pocket Conquest rules engine (Python) implementing the rules above, plus a
   tiny HTTP service. The engine must compute the legal moves for a position and validate a
   proposed move. Name any dependency you install (e.g. a validation library) — do not add an
   unnamed one.
2. The service: POST with JSON `{"position": {...}, "moves": [...]}` (board, gold, armies,
   side to move, and the legal moves for the side to move) returns JSON `{"move": "<move>"}`.
   The move format is yours to define (state it in the reply and in the code README) — it
   must be unambiguous (which stack, from where, to where / build where, attack or move).
3. The service calls DeepSeek with the same model and key Hermes already uses for his own
   work. The key stays under `~/.hermes/` (Hermes knows the exact path) and never appears in
   code, the task, or the staged reply. The prompt MUST carry, every move: the full rules
   (condensed but complete), the current board + gold + armies, AND the list of legal moves —
   ask for exactly one move from that list and nothing else; low temperature, small max
   tokens. Validate the returned move is legal for the given position with the rules engine
   before answering. If the reply is illegal or not in the list, retry once or twice,
   sending the error back with the position and the legal list. If it still fails, return
   HTTP 500 with `{"error": "..."}` so the page falls back to the built-in computer. Never
   return an illegal move as if it were good, and never answer a random legal move.
4. Keep the whole round trip inside the page's 30-second budget: short prompt, small max
   tokens, and a client-side timeout around 6 seconds so a slow model call fails fast
   instead of hanging. Note: the model has never seen this game, so the prompt is longer
   than the chess bridge's — keep it as tight as the rules allow and report the per-move
   token counts.
5. Run the service on the laptop now and record the exact start/stop commands. Say plainly
   whether it survives a laptop reboot or needs a manual restart — do not claim reboot
   survival you have not proven.
6. Expose it via Tailscale Serve on an HTTPS path (for example
   `https://home.tail2bff3f.ts.net/pocket-conquest-move`), tailnet-only, no OAuth. Record
   the exact URL Stephen pastes into the page's agent box. If Serve needs root (as it did
   for 0268 — no operator set on this host), record the exact sudo command and the URL in
   the reply and mark the Serve step as blocked pending Stephen; do not work around it.
7. Copilot peer review, locked lane (this is a backend API parsing network input): the
   reviewer answers the 7-item security checklist (no hardcoded secrets, inputs
   bounds-checked, auth on every endpoint, TLS off-box, least privilege, fail-closed
   errors, pinned deps), pass/fail each. Fix any fail before staging.
8. Commit the code to cabin-play (play project — never nordtronics). Push via SSH; gh.py
   cannot write to cabin-play.
9. Prove it: curl the Serve URL (or localhost if Serve is blocked) with a test position
   and show a legal move inside the budget. Include at least one position where the model
   must choose between building, moving, and attacking, and show the move is legal.

## Success criteria

- Rules engine implements the rules above (legal-move generation + move validation), proven
  by the curl proof and by a unit check on the initial position.
- Service running on the laptop; exact Serve URL recorded (or blocked-with-command recorded).
- Curl proof shows a legal move for a test position, inside the budget.
- Copilot locked-lane review done (SHIP, or findings fixed).
- Code committed to cabin-play.
- Start/stop commands recorded; reboot survival stated honestly.

## Constraints

- Cost: one short DeepSeek completion per move, no long reasoning. Name the model used.
  Report per-move token counts (prompt in / completion out) in the reply.
- The DeepSeek key never leaves `~/.hermes/` and never appears in code, the task file, or
  the staged reply.
- The rules above are fixed — do not invent, change, or simplify them. If a rule is
  ambiguous in play, record the reading you used and flag it in the reply.
- Never break the page: on any failure the endpoint errors out and the built-in computer
  takes over.
- Do not reboot the laptop — 0262/0264 own the reboot.
- The Serve path carries no OAuth (the page cannot log in). Tailnet only is fine —
  Stephen's phone is on the tailnet.

## Proof

- Serve URL + curl transcript showing a legal move inside the budget.
- Cabin-play branch/commit + Copilot locked-lane verdict.
- Per-move token counts.

## Reply format

Short report: Serve URL (or blocked command), start/stop commands, curl proof, Copilot
verdict, per-move token counts, reboot-survival statement.

## Assignment

Hermes routes the build to Newton via Herdr; Newton builds. Copilot peer-reviews (locked
lane, GitHub-bound — code goes to cabin-play). Juno verifies independently.
