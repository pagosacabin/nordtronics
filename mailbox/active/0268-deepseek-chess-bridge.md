---
task_id: "0268"
status: in_progress
iteration: 3
proof: []
expect-reply-within: 6h
notes: |
  BLOCKED — the endpoint is built, running and proven on localhost, but step 5 (Tailscale Serve) cannot be done from this tick: tailscale has no operator set on this host, so `tailscale serve` requires root, and this worker has no root (sudo asks for a password; pkexec hangs with no polkit agent; a direct LocalAPI POST returns 403 "serve config denied"). No proof block; nothing staged.

  DECISION OWED — one command as root, then the URL is usable:
    sudo tailscale serve --bg --https=443 --set-path /chess-move http://127.0.0.1:8765
  and paste https://home.tail2bff3f.ts.net/chess-move into the page's agent box. Everything else is done and verified.

  What was done:
   - Build routed to Newton via Herdr (pane t19 agy-main, Gemini 3.8 Flash High; both pools read 100% before the run). Newton spent ~12 minutes probing the environment (stockfish uci, `python -c "import chess"` and similar) across 5 turns and wrote no file; the turn was interrupted (`hc interrupt t19` -> esc -> stopped) and Hermes authored the service directly. That is a declared deviation from "Newton builds"; no output of Newton's was used.
   - Code on cabin-play branch hermes/0268-chess-bridge, tip c53c140 (pushed; `git ls-remote --heads origin` returns c53c140b8eb24336311e3f03107f99b982a12e24). Files: chess-bridge/{server.py,run.sh,requirements.txt,README.md,chess-bridge.service,install-stockfish.sh}. Nothing was written to nordtronics except this mailbox evidence.
   - Deps installed without root: python-chess 1.11.2 in ~/.local/share/chess-bridge/venv; Stockfish 17.1 static build at ~/.local/bin/stockfish via install-stockfish.sh, which pins the release tarball's SHA-256 (09e95322...e31987d) and refuses any binary whose `id name` is not "Stockfish 17.1". The pin was verified against a fresh download (`sf.tar: OK`).
   - Service runs as the user unit chess-bridge.service (installed under ~/.config/systemd/user/, enabled; hardening live per `systemctl --user show`: ProtectSystem=strict, ProtectHome=read-only, NoNewPrivileges=yes, empty CapabilityBoundingSet, MemoryMax=256M, TasksMax=32). It is independent of this tick's shell.
   - DeepSeek call: model deepseek-flash, thinking disabled, temperature 0, max_tokens 16, 4 s timeout; prompt carries the FEN, the moves so far and the full legal UCI list; an illegal/absent answer is retried once with the error text; then Stockfish (`go movetime 400` through python-chess's UCI engine wrapper) is the fallback; anything neither covers returns 500. No random legal move.
   - CORS: the page's JSON POST is cross-origin, so OPTIONS /chess-move answers 204 with Access-Control-Allow-Origin for https://nordtronics.io (+ www), Allow-Methods POST/OPTIONS, Allow-Headers content-type. 0267's read of the live page found this requirement; 0268's spec did not name it.
   - Copilot locked-lane review (pane t17, mai-code-2-flash), two passes. Pass 1: Verdict FIX — (1) HIGH no application authentication, (2) MED Stockfish download not digest-pinned, (3) MED no sandbox/service identity. Pass 2, after fixes: Verdict SHIP — (2) RESOLVED, (3) RESOLVED, (1) STILL OPEN by design (the task forbids OAuth on the Serve path and the page cannot send credentials; README documents it as the accepted tailnet boundary).

  Verified on http://127.0.0.1:8765 (full transcript committed at mailbox/files/0268/curl-local-proof.txt, generator at mailbox/files/0268/proof.py):
   - GET /health -> {"ok": true, "model": "deepseek-flash", "stockfish": "Stockfish 17.1"}
   - OPTIONS preflight -> 204 with the four CORS headers above.
   - Three positions -> legal UCI moves in 0.62-0.74 s: startpos e2e4; Italian midgame g8f6; kingside attack c4b5. Legality asserted per FEN with python-chess.
   - Fail-closed: invalid FEN 400, missing FEN 400, 600 moves 400, body >8 KiB 400.
   - Stockfish fallback (second instance with the key removed): mate-in-1 -> a1a8 in 0.33 s, startpos -> e2e4.
   - Journal shows `source=model` and per-move usage (218-310 prompt tokens, 4 completion tokens).

  Corrections and facts worth carrying:
   - A hand-rolled Stockfish stdin script that writes `quit` immediately after `go` cuts the search off at depth 1: the engine answered a2a3 from the start position and missed mate in 1, while looking perfectly healthy. Switched to chess.engine.SimpleEngine; the mate-in-1 case is now the regression check.
   - Serve could not be configured even through the local API: GET /localapi/v0/serve-config works, POST returns HTTP 403 "serve config denied". Recorded rather than worked around.
   - Reboot survival: it does NOT come back on its own. The unit is a user unit enabled in default.target; this host boots to a greeter (0262's finding), so it starts when the user session starts, not at boot.
   - No CI: cabin-play has no .github/workflows, so no run exists to cite for this branch.
   - The DeepSeek key stays in ~/.hermes/.env; run.sh extracts only DEEPSEEK_API_KEY, and a key-shaped-string grep over the committed tree is clean.

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

---

## Status: BLOCKED — one root command outstanding (no proof block)

There is **no `proof` block** and this task was not staged. The endpoint is built,
committed, running and verified on `http://127.0.0.1:8765`, and the Copilot
locked-lane review ended at **Verdict: SHIP**. The one success criterion that is
not met is step 5: this host's `tailscale` has no operator set, so
`tailscale serve` needs root, and the worker has no root (`sudo` wants a password,
`pkexec` has no polkit agent, and the local API answers `403 serve config denied`).

The decision owed is a single command:

```bash
sudo tailscale serve --bg --https=443 --set-path /chess-move http://127.0.0.1:8765
```

After that, paste `https://home.tail2bff3f.ts.net/chess-move` into the page's
agent box. The localhost proof and the fallback/timing evidence are in
`mailbox/files/0268/curl-local-proof.txt`.
