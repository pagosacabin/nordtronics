---
task_id: "0267"
title: How would Hermes serve a chess-move endpoint?
status: staged
iteration: 2
expect-reply-within: 24h
claimed_by: "cronrunner/c0be50a686c6"
claimed_at: "2026-11-27T14:47:01+00:00"
lease_until: "2026-11-27T16:17:01+00:00"
proof:
  branch: none
  sha: none
  run: none
  files: []
  reply_is_the_deliverable: mailbox/staged/0267-hermes-chess-endpoint.md
notes: |
  Host-only research task: no repo file changed, no branch created, no CI run applies.
  `run: none` is earned by enumerating the workflows, not asserted: `.github/workflows/`
  holds android-build.yml (push: branches [android-toolchain-setup]),
  android-companion-v0.yml (push: a fixed branch allow-list + `android/**` paths),
  backend.yml (`backend/**` paths), detection-sim.yml (`python/detection-sim/**`),
  platformio.yml (`firmware/tank-monitor|node-v1|wildfire-node-v1`, `python/detection-sim/**`)
  and website-check.yml (push: branches [main, hermes/0068…, hermes/0070…, hermes/0093…]).
  None can fire for a task that changes no file, and no branch was pushed, so no run exists
  to cite. `files: []` because the only artifact is this reply.

  What was done, all read-only on the host plus 8 metered model calls:
   - Inventoried what can serve HTTP here: hermes-gateway hermes-gateway.service (up 1w0d,
     pid 152866) listening on 127.0.0.1:9120 only; tailscale serve publishes
     https://home.tail2bff3f.ts.net/ -> http://127.0.0.1:9120 (tailnet only, funnel off);
     Tailscale node 100.78.85.106 "home"; tailnet peers muse, homeassistant, stephens-a35.
   - Confirmed the gateway cannot be the endpoint: GET / on 127.0.0.1:9120 AND through the
     serve URL both answer 302 -> /auth/login?provider=nous&next=%2F (uvicorn, "vary: Origin").
     It is a Nous-Portal-gated UI, so an anonymous browser POST cannot reach a route there.
   - Read the real client contract out of the live page (curl https://nordtronics.io/chess/,
     52,857 B): aiTurn() calls fetch(url,{method:"POST",headers:{"Content-Type":
     "application/json"},body:{fen,moves}}) with an AbortController at 6000 ms, then expects
     {move:"<uci>"}; an empty/unreachable/illegal answer silently falls back to the built-in
     computer. No Authorization header is ever sent, and application/json makes this a
     CORS-preflighted cross-origin request, so the server must answer OPTIONS with
     Access-Control-Allow-Origin for https://nordtronics.io (or *) and Allow-Headers: content-type.
   - Verified the TLS the browser would see: openssl s_client to home.tail2bff3f.ts.net:443
     returns CN=home.tail2bff3f.ts.net, issuer Let's Encrypt YE2, valid 2026-11-11..2027-02-09.
     So the serve URL is HTTPS and not mixed content.
   - Measured the move producer against the 6 s budget on one real midgame FEN
     (r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 4), deepseek-flash,
     8 inference calls total. THINKING MODE (the API default) is unusable: capped at 16 and
     600 tokens it spent every token on reasoning and returned empty content in 0.67/1.05 s
     and 3.26/3.79 s; uncapped it answered "g8f6" only after 2,912 reasoning tokens, 16.39 s.
     NON-THINKING mode (thinking:{type:"disabled"}) answered in 0.71 s and 1.01 s but replied
     "Nf6" — SAN, not the UCI the page requires. With the LEGAL_MOVES list in the prompt it
     answered "g8f6" in 0.64 s (215 in / 4 out tokens).
   - Checked the missing pieces for that shape: neither python-chess nor stockfish is
     installed (`import chess` ModuleNotFoundError on /usr/bin/python3 and on the Hermes tool
     python; `rpm -q stockfish` not installed, `which stockfish` empty). fastapi+uvicorn DO
     import in the Hermes tool python 3.14.9, and 87xx is free on the host.
   - No port was opened, no service started, no config changed, no repo file touched. The only
     thing written is a throwaway timing harness at
     ~/.hermes/profiles/cronrunner/cache/scratch/0267/latency_probe.py (worker scratch, never
     committed) — declared so the 8 calls are attributable.

  Cost below covers the 8 probe calls only; this worker session's own token usage is not
  exposed to the harness, so it is not included rather than estimated.
cost:
  model: deepseek-flash
  api_calls: 8
  input_tokens: 1048
  input_cache_hit_tokens: 0
  input_cache_miss_tokens: 1048
  output_tokens: 4159
  total_tokens: 5207
  search_requests: 0
  rate_window: off-peak
  rates_used: "$0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: 0.00265
---

# 0267 — reply: a tailnet-served POST endpoint, with the model in non-thinking mode

**Status: recommendation, not a build.** Nothing was written, started or exposed.

## Mechanism

A small dedicated HTTP service on loopback, published at a path by Tailscale Serve:

```
127.0.0.1:8766  (FastAPI/uvicorn, ~60 lines)
        ↑
tailscale serve --bg --set-path /chess-move 127.0.0.1:8766
        ↑
https://home.tail2bff3f.ts.net/chess-move   (tailnet only)
```

Not the Hermes gateway: `GET /` on 127.0.0.1:9120 and through the serve URL both answer
`302 → /auth/login?provider=nous&next=%2F`. It is an OAuth-gated UI; there is no anonymous
route to hang a chess move on. Serve is the right publisher because it already fronts `/`
→ 9120 on the same hostname, terminates TLS with a real Let's Encrypt certificate
(`CN=home.tail2bff3f.ts.net`, issuer YE2, valid to 2027-02-09), and adding a second path
handler leaves the `/` handler alone.

## URL shape and auth

- Paste `https://home.tail2bff3f.ts.net/chess-move` into the "Agent endpoint URL" box.
  HTTPS, because the page is HTTPS and a plain `http://` endpoint is mixed-content blocked.
- **Tailnet only** (`tailscale serve status` reports "tailnet only"; funnel is off). It works
  from the laptop, `stephens-a35`, and anything else joined to the tailnet. A visitor off the
  tailnet gets a failed fetch and the page plays its built-in computer instead — no error
  shown, which is the page's designed fallback.
- **No header auth is possible.** The page's own `fetch` sends only `Content-Type:
  application/json`; there is nowhere to put a bearer token. Tailnet membership *is* the
  access control. If it ever needs to be public, `tailscale funnel` on that path makes it an
  unauthenticated internet endpoint, and the only credential you could carry is in the URL
  itself (a random path segment Stephen pastes) — worth knowing before opening it.
- **CORS is mandatory and easy to forget.** The page is `https://nordtronics.io`; the POST is
  cross-origin and `application/json` is not a CORS-safelisted content type, so the browser
  sends a preflight. The service must answer `OPTIONS` with
  `Access-Control-Allow-Origin: https://nordtronics.io` (or `*`, since no credentials are
  sent) and `Access-Control-Allow-Headers: content-type`.

## How a move gets produced

A single DeepSeek call per move — but **only with thinking mode disabled**, and only against
a legal-move list. Measured on `rnbqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq -`:

| call shape | latency | answer |
|---|---|---|
| thinking (API default), 16 or 600 output tokens | 0.67 / 1.05 s, 3.26 / 3.79 s | **empty** — all tokens went to reasoning |
| thinking, uncapped | **16.39 s** (2,912 reasoning tokens) | `g8f6` |
| non-thinking, no move list | 0.71 / 1.01 s | `Nf6` — SAN, not UCI |
| non-thinking + LEGAL_MOVES list | **0.64 s** (215 in / 4 out) | `g8f6` ✓ |

So the model is a fine producer (~0.7 s) once it is told the legal moves and not to think;
the default reasoning path is 16 s and would always lose to the 6-second abort.

## Against the 6-second budget

Comfortable, at roughly **0.7-1.0 s** end to end once thinking is off: move generation
(<50 ms), one non-thinking completion (~0.65 s), tailnet RTT, JSON round trip. The slow step
by a factor of twenty is thinking mode, which is on by default — the one thing that must be
set explicitly (`"thinking": {"type": "disabled"}` in the request body) or the endpoint will
look broken. Cost per move is ~219 tokens ≈ $0.00003 off-peak.

## What would need building (no code written)

1. `python3 -m pip install python-chess` — not installed on this host (`import chess` fails,
   and `stockfish` is not installed either). It is what turns FEN into the legal UCI list and
   validates the reply; without it the endpoint cannot guarantee a legal move, and an illegal
   one silently drops the game back to the built-in computer.
2. The service itself: `POST /chess-move` → parse `{fen, moves}` → legal moves from
   python-chess → one non-thinking DeepSeek call with that list → validate the returned UCI
   against it → `{"move": "<uci>"}`; on any doubt return an empty/invalid body so the page
   falls back rather than playing something illegal. Plus the CORS preflight handler above.
3. `tailscale serve --bg --set-path /chess-move 127.0.0.1:8766` and a systemd **user** unit
   for the app (the gateway's own unit is the model — enabled under `linger`, so it survives
   a logout).
4. Optional strength upgrade: `dnf install stockfish` and answer from UCI at a fixed
   `go movetime 1500`, using the model only for commentary. Stronger play, no reasoning-token
   risk, same latency envelope.

## Blocker, stated plainly

There is **no publicly usable shape from this host today.** Hermes's only published URL is
the OAuth-gated UI, and a tailnet-only endpoint is reachable only from Stephen's own devices.
A public opponent has to live where the page's own `/chess/api/*` already lives — on the
nordtronics server — which means either running the move service there, or the poll-based
design the Juno game already uses (page posts, agent answers minutes later). The 6-second
budget rules out the agent-loop version of Hermes answering at all: this reply's own ticks
are measured in minutes, not seconds.
