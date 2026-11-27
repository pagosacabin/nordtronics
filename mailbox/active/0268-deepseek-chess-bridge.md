---
task_id: "0268"
status: in_progress
iteration: 3
proof: []
notes: "Amended 2026-11-26 (iteration 2, before pickup): prompt now sends the legal move list; retry with the error; server-side Stockfish fallback instead of a bare 500. Per Stephen's word after Claude's architecture review."
expect-reply-within: 6h
---

# 0268 — DeepSeek chess-move bridge (interim opponent)

## Context

Stephen's chess page (https://nordtronics.io/chess/) has a "Plug in your
own agent" box. The page calls the endpoint directly from the browser:
POST `{"fen": "…", "moves": ["e2e4", …]}` (UCI moves), expects
`{"move": "e7e5"}` back, 6-second timeout, and falls back to the
built-in computer if the endpoint is empty, unreachable, or illegal.

While 0267 answers how Hermes himself would serve such an endpoint, the
interim opponent is DeepSeek. Hermes already uses DeepSeek (his own key
and model, under `~/.hermes/`). The bridge must be reachable from
Stephen's phone, whose browser loads the page over HTTPS — so the
endpoint must be HTTPS too, or the browser blocks it as mixed content.
Tailscale Serve can expose a localhost service at
`https://home.tail2bff3f.ts.net/<path>` with a valid cert; Stephen's
phone is on the tailnet. The page's fetch cannot log in, so the Serve
path must carry no OAuth.

## Task

1. Route the build to Newton via Herdr (pure software; bench firmware
   stays with Hermes). Newton builds a tiny HTTP service, Python stdlib
   only unless a dependency is named in the reply:
   - `POST /` (or one fixed path) with JSON `{"fen": "…", "moves": […]}`
     returns JSON `{"move": "<uci>"}`.
   - On any failure Stockfish cannot cover, return HTTP 500 with
     `{"error": "…"}` so the page falls back to the built-in
     computer. Never return an illegal move as if it were good, and
     never answer a random legal move.
2. The service calls DeepSeek with the same model and key Hermes
   already uses for his own work. The key stays under `~/.hermes/`
   (Hermes knows the exact path) and never appears in code, the task,
   or the staged reply. Prompt shape: the FEN, the moves so far, AND
   the list of legal moves (UCI) for the position — ask for exactly
   one move from that list and nothing else; low temperature, small
   max tokens. Validate the returned move is legal for the given FEN
   before answering (python-chess or equivalent — name it if you
   install one). If the reply is illegal or not in the list, retry
   once or twice, sending the error back with the position and the
   legal list. If it still fails, fall back server-side to Stockfish:
   play Stockfish's best move for the position. A random legal move
   is never the fallback — it can hang a piece.
3. Keep the whole round trip inside the page's 6-second budget: short
   prompt, small max tokens, and a client-side timeout around 4 seconds
   so a slow model call fails fast instead of hanging.
4. Run the service on the laptop now and record the exact start/stop
   commands. Say plainly whether it survives a laptop reboot or needs a
   manual restart — do not claim reboot survival you have not proven.
5. Expose it via Tailscale Serve on an HTTPS path (for example
   `https://home.tail2bff3f.ts.net/chess-move`), tailnet-only, no OAuth.
   Record the exact URL Stephen pastes into the page's agent box.
6. Copilot peer review, locked lane (this is a backend API parsing
   network input): the reviewer answers the 7-item security checklist
   (no hardcoded secrets, inputs bounds-checked, auth on every
   endpoint, TLS off-box, least privilege, fail-closed errors, pinned
   deps), pass/fail each. Fix any fail before staging.
7. Commit the code to cabin-play (play project — never nordtronics).
   Push via SSH; gh.py cannot write to cabin-play.
8. Prove it: curl the Serve URL from the laptop with a test position
   and show `{"move": "<legal UCI move>"}` inside 6 seconds.

## Success criteria

- Service running on the laptop; exact Serve URL recorded.
- Curl proof shows a legal UCI move for a test position, inside 6
  seconds.
- Copilot locked-lane review done (SHIP, or findings fixed).
- Code committed to cabin-play.
- Start/stop commands recorded; reboot survival stated honestly.

## Constraints

- Cost: one short DeepSeek completion per move, no long reasoning.
  Name the model used.
- Stockfish is a named dependency: install it on the laptop (name the
  package) and use it as the server-side fallback in step 2. If
  Stockfish is unavailable, return HTTP 500 and let the page fall
  back to the built-in computer.
- The DeepSeek key never leaves `~/.hermes/` and never appears in
  code, the task file, or the staged reply.
- Never break the page: on any failure the endpoint errors out and the
  built-in computer takes over.
- Do not reboot the laptop — 0262/0264 own the reboot.
- The Serve path carries no OAuth (the page cannot log in). Tailnet
  only is fine — Stephen's phone is on the tailnet.

## Proof

- Serve URL + curl transcript showing a legal UCI move inside 6
  seconds.
- Cabin-play branch/commit + Copilot locked-lane verdict.

## Reply format

Short report: Serve URL, start/stop commands, curl proof, Copilot
verdict, reboot-survival statement.

## Assignment

Hermes routes the build to Newton via Herdr; Newton builds. Copilot
peer-reviews (locked lane, GitHub-bound — code goes to cabin-play).
Juno verifies independently.
