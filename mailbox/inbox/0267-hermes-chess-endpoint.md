---
task_id: "0267"
status: filed
iteration: 1
proof: []
notes: ""
expect-reply-within: 24h
---

# 0267 — How would Hermes serve a chess-move endpoint?

## Context

Stephen's chess page (https://nordtronics.io/chess/) has a "Plug in your
own agent" box. The page calls the endpoint directly from the browser:

1. It sends a POST with JSON: `{"fen": "…", "moves": ["e2e4", …]}`
   (moves in UCI format).
2. It expects JSON back: `{"move": "e7e5"}` (one move, UCI format).
3. It gives the endpoint 6 seconds; if the endpoint is empty,
   unreachable, or answers with an illegal move, the built-in computer
   plays instead.

Stephen wants Hermes as a chess opponent. Hermes's only known URL is his
webapp, served over Tailscale Serve behind Nous Portal OAuth — a
login-gated UI, not an endpoint the page can POST to. This task asks
Hermes how he would serve a real move endpoint. Research and
recommendation only — build nothing yet. The interim opponent (DeepSeek
bridge) is task 0268.

## Task

Answer, from what is actually on your machine: how would you serve a
FEN-in / UCI-move-out endpoint the chess page can call?

1. Check what you can run: your gateway, your CLI, a small HTTP server
   of your own, Tailscale Serve. Name the mechanism you would use and
   why.
2. Give the concrete shape: the URL the page would paste in (including
   whether it is HTTPS — the page is HTTPS, so plain `http://` will be
   blocked by the browser — and whether it is tailnet-only), and what
   auth, if any, sits in front of it.
3. Say how a move gets produced: your own reasoning over the position,
   a model call, or something else. Name the model if a model call is
   involved.
4. Judge it against the 6-second budget: can you answer inside 6
   seconds, or would the page usually fall back to the built-in
   computer? Say what the slow step is.
5. If no workable shape exists, say so plainly and name the blocker —
   do not fake it.

## Success criteria

- Staged reply with a concrete recommendation: mechanism, exact URL
  shape, how moves are produced, latency vs the 6-second budget, and
  what would need building — or a plain statement that there is no
  workable shape, with the blocker named.
- No code written, no config changed, no service started.

## Constraints

- Read-only research. Do not build anything, do not change any config,
  do not start or expose any service.
- Do not put any secret (API key, token, password) in the task file or
  the staged reply. Name key locations by path only.
- Keep any model calls to the minimum needed to answer the latency
  question.

## Proof

- Staged reply with the recommendation. Host-only; no branch, no push.

## Reply format

Short report: recommended mechanism, URL shape, how moves are produced,
latency vs 6 seconds, what needs building (or the blocker).

## Assignment

Hermes does this himself on the laptop. No Newton (no build), no
Copilot (not GitHub-bound, no repo change).
