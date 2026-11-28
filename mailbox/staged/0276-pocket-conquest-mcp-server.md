---
task_id: "0276"
title: "Pocket Conquest MCP server (agent plays via tools)"
status: staged
iteration: 2
expect-reply-within: 24h
proof:
  branch: hermes/0276-pocket-conquest-mcp
  repo: pagosacabin/cabin-play
  sha: f67e64bb4d2fba7e6e47baaaa91e7e1497ea614e
  run: none
  run_note: >-
    cabin-play carries no `.github/` at all, so no workflow can trigger on this branch:
    `git ls-tree -r --name-only f67e64b | grep -i '^\.github'` is empty, there is no
    `.github/workflows` directory in the tree, `gh api repos/pagosacabin/cabin-play/contents/.github`
    answers 404, and `gh run list -R pagosacabin/cabin-play --branch hermes/0276-pocket-conquest-mcp`
    returns nothing. No ntfy receipt either: there is no compiled artifact (a Python MCP server
    committed to cabin-play) and the task names no topic.
  copilot_review: >-
    NOT AVAILABLE -- no Copilot verdict exists. Pane t17 (copilot-main) answers "You have
    exceeded your monthly quota (Request ID: 00000-c928d8c3-4fce-4d5f-8036-13995e3ba537)" and
    "You've used all your Copilot Free chat requests for the month. Your quota resets on Nov 30
    at 5 PM" at turn 342 / Session 138.23 AIC used (mailbox/files/0276/copilot-quota.txt).
    SUBSTITUTE, declared as a deviation: Antigravity, 3 passes, transcripts committed.
    Pass 1 (Claude Sonnet 4.6): 7-item checklist with one FAIL (deps not visible to the
    reviewer) and 7 findings, verdict FIX. Pass 2 (Claude Sonnet 4.6): all 7 pass-1 items
    RESOLVED, 3 NEW findings, verdict FIX. Pass 3 (Gemini 3.8 Flash -- the Claude/GPT pool
    hit 0% between passes, `RESOURCE_EXHAUSTED ... Resets in 163h26m56s`): all 3 pass-2 items
    RESOLVED, no new HIGH/MEDIUM, verdict SHIP. A Copilot locked-lane pass over
    f67e64b is owed when its quota resets on Nov 30.
  files:
    - pocket-conquest-mcp/server.py
    - pocket-conquest-mcp/local_api.py
    - pocket-conquest-mcp/tests/mcp_session_proof.py
    - pocket-conquest-mcp/README.md
    - pocket-conquest-mcp/requirements.txt
    - pocket-conquest-mcp/.gitignore
  evidence:
    - mailbox/files/0276/mcp-session-proof.txt
    - mailbox/files/0276/newton-build-route.txt
    - mailbox/files/0276/copilot-quota.txt
    - mailbox/files/0276/review-pass1-antigravity-claude-sonnet-4-6.txt
    - mailbox/files/0276/review-pass2-antigravity-claude-sonnet-4-6.txt
    - mailbox/files/0276/review-pass3-antigravity-gemini-3.8-flash-high.txt
    - mailbox/files/0276/antigravity-usage-after.txt
    - mailbox/files/0276/api-contract-divergence.txt
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
  rates_used: "not applicable -- no figure is quoted"
  cost_usd: unknown
  cost_note: >-
    The worker harness exposes no per-call usage objects to this session, so no token or cost
    figure is estimated here (protocol: unknown rather than a guess). The deliverable itself
    makes ZERO model calls: the MCP server is tools + the 0275 rules engine, and the stand-in
    API is a stdlib HTTP server. The only model calls in the task were the build route
    (Newton/agy, 65,381 input + 7,725 output tokens, JSON in
    mailbox/files/0276/newton-build-route.txt) and the three substitute reviews
    (mailbox/files/0276/review-pass*.txt headers carry their usage), all on the Antigravity
    free tier, not on a paid DeepSeek key.
notes: |
  DELIVERED on the code and the end-to-end proof. One gate is open and stated plainly: the
  Copilot locked-lane pass (quota resets Nov 30); Antigravity substituted for it with a SHIP.

  WHAT SHIPPED. cabin-play:pocket-conquest-mcp/ on branch hermes/0276-pocket-conquest-mcp,
  tip f67e64b, branched from 0275's tip 08efcd2 so the rules engine is imported in place
  (../pocket-conquest-bridge/pocket_conquest.py) rather than copied or rebuilt:
    server.py              MCP server over stdio (mcp 2.0.0, MCPServer), three tools.
    local_api.py           local stand-in for the game API (loopback only, mode-0700 store),
                           authoritative: it re-validates every submitted move with the engine.
    tests/mcp_session_proof.py  spawns the stand-in, connects a real MCP client over stdio and
                           runs the whole session plus the refusals; exit 0 on 12/12.
    README.md              full rules, move grammar, tool docs, API contract.
    requirements.txt       mcp==2.0.0 (single pinned dep).
    .gitignore             __pycache__.

  THE THREE TOOLS. get_game(game_id="") creates a game when the id is empty (so a cold agent can
  start from one call) or reads an existing one; it returns board text, per-tile armies, gold,
  side to move, turn, the full legal move list and a rules summary. list_legal_moves(game_id)
  recomputes the engine's list. make_move(game_id, move) validates with the engine, refuses
  illegal / wrong-side / finished-game / malformed-id input with a clear error, POSTs, then
  RE-READS the game and only reports success if the stored position is the one the engine
  computed. Every tool description carries the rules summary, the grid, the side and the move
  grammar, so a model that has never seen the game can play from the descriptions plus the
  legal-move list.

  MOVE GRAMMAR is 0275's (documented in the README): "B <tile> <n>", "M <src> <dst> <n>",
  "M <src> <dst> <n> + <src2> <n2>", "END". One rules reading worth repeating: a move does not
  hand the turn over -- END does -- which is why the proof shows side_to_move staying "S" after
  M B5 B4 1 and flipping to "R" after END.

  WHO BUILT IT. The route was Newton via Herdr as instructed (pure software). `agy -p` with
  claude-sonnet-4-6 returned status SUCCESS in 202 s / 1 turn and wrote
  newton/pocket-conquest-mcp/server.py (65,381 in / 7,725 out, 410,366 cache-read;
  mailbox/files/0276/newton-build-route.txt). I read its output against a proof harness I wrote
  and corrected four things, all committed:
    1. its "confirmation digest" hashed only turn/side/army counts, which cannot see an owner
       change, so a claimed tile could confirm on unchanged counts -- replaced with a canonical
       whole-position digest;
    2. it had no optimistic-concurrency token, so a move built on a stale copy would overwrite
       the newer state -- added `from_digest` (the API refuses a stale state with 409);
    3. make_move answered "in_progress" while get_game answered "active" for the same state --
       unified on active/finished;
    4. the illegal-move error did not name the side to move, so a wrong-turn move read as an
       outright illegal one -- it now says which side is to move.
  The stand-in, the proof harness, the README and the fixes are mine; every number below is
  from code I ran.

  PROOF, run today against the stand-in on loopback with this exact tree
  (mailbox/files/0276/mcp-session-proof.txt), 12/12 checks, exit 0:
    initialized: server=pocket-conquest protocol=2025-11-25
    get_game('') creates a test game                     -> 89174a3ef690bbbe
    get_game(id)  side_to_move=S, gold 10/10, armies+board present
    list_legal_moves 12 moves, identical to the engine's own list
    make_move('M B5 B4 1') accepted; the move appears in the game JSON and the stored position
       hashes equal to the position the engine produced
    END collects income (gold 10 -> 17) and passes the turn to R
    refusals, each with nothing written: illegal move; a move for the side not to move;
       a move on a finished game (S has taken E1); a malformed move; a malformed game id
       ('../../etc/passwd')
  Test games lived in a scratch store under the worker's TMPDIR and are DELETED; no game id
  survives this run.

  REVIEW (substitute, Copilot unavailable -- quota until Nov 30):
    pass 1 (Claude Sonnet 4.6): checklist 6 PASS / 1 FAIL, 7 findings, verdict FIX. Findings
      were: no move-length cap before is_legal; plain http allowed for RFC-1918; client-supplied
      `position` forwarded in the POST; /api/new body left unread (keep-alive misframing); /tmp
      store world-readable; silent fallback to the stub; partial-success dict on normalisation
      failure. The deps FAIL was a scope artefact -- requirements.txt exists and pins
      mcp==2.0.0; it was not in the reviewer's read list.
    pass 2 (Claude Sonnet 4.6): all 7 RESOLVED; 3 NEW findings (README still documented the
      dropped `position`; the fallback warning fired per call; serve()'s default store still
      named /tmp). Verdict FIX.
    pass 3 (Gemini 3.8 Flash, because the Claude/GPT pool hit 0% between passes): all 3
      RESOLVED, no new HIGH/MEDIUM, verdict SHIP.
  Every fix is in the tree at f67e64b and the proof above was re-run after them. Declared as a
  deviation: the third pass is a weaker model than the first two, and no Copilot verdict exists.

  DECLARED: the API contract two tasks name is not the same contract. 0276 names
  POST /pocket-conquest/api/new, POST /pocket-conquest/api/move and GET
  /pocket-conquest/games/<id>.json. 0279 (filed the same morning, follow-up to 0275) says the
  LIVE page calls POST {base}/games, POST {base}/games/{id}/move and GET {base}/games/{id}, and
  that the /games route does not exist yet. I built to 0276's stated shape, as the task says to
  do without guessing, and recorded the divergence with the exact page-contract quote in
  mailbox/files/0276/api-contract-divergence.txt. If the VPS API is built to the page's shape,
  only the three URL constants and the response field names in server.py move; the engine,
  validation, refusal paths and proof harness are unaffected. This is 0279's contract to settle,
  not something I should decide unilaterally.

  WHAT IS NOT DONE, stated plainly: (a) the live VPS API is Juno's to stand up -- the proof runs
  against a local stand-in implementing the documented contract, exactly as 0276 allows, and
  names the live API as the remaining piece; (b) no Copilot locked-lane verdict (quota);
  (c) this run did not touch nordtronics, the VPS, the Serve path or the page.
---

# 0276 — Pocket Conquest MCP server: delivered

**An agent can now play Pocket Conquest through three MCP tools over stdio.** Rules legality comes
from 0275's engine, imported in place; nothing about the game was re-implemented.

## Start it

```bash
# 1. the game API stand-in (test harness -- the live VPS API is Juno's)
python3 local_api.py --port 8791

# 2. any MCP client spawns the server over stdio
POCKET_CONQUEST_API_BASE=http://127.0.0.1:8791 python3 server.py
```

`POCKET_CONQUEST_API_BASE` must be `https://` off-box; plain `http` is accepted only for
loopback, so the test path cannot silently become the production one.

## The three tools

| Tool | One line |
|---|---|
| `get_game(game_id="")` | Empty id creates a game; otherwise reads it. Returns board text, armies, gold, side to move, the legal list and the rules. |
| `list_legal_moves(game_id)` | The engine's complete legal move list for the side to move. Authoritative. |
| `make_move(game_id, move)` | Validate with the engine, submit, re-read the game, confirm the stored position is the one the engine computed. |

Every description carries the grid, the side you play (S, Nordhold), the move grammar and a rules
summary, so a model calling them cold can play legally.

## Proof (12/12, exit 0)

```
initialized: server=pocket-conquest protocol=2025-11-25
[PASS] three tools advertised -- get_game,list_legal_moves,make_move
[PASS] tool descriptions carry the rules
[PASS] get_game('') creates a test game -- 89174a3ef690bbbe
[PASS] get_game(id) reports side to move, gold, armies and the board -- side_to_move=S gold={'S': 10, 'R': 10}
[PASS] list_legal_moves matches the engine exactly -- 12 moves, engine 12
[PASS] make_move('M B5 B4 1') lands in the game JSON
[PASS] the API-side state is the position the engine produced -- digest equal
[PASS] an illegal move is refused and nothing is stored
[PASS] END collects income and passes the turn to R
[PASS] a move for the side not to move is refused
[PASS] a move on a finished game is refused
[PASS] malformed move and malformed game id are refused
12/12 checks passed
```

Full transcript: `mailbox/files/0276/mcp-session-proof.txt`. The test games were scratch data and
are deleted.

## Review

Copilot's locked lane is unavailable (monthly quota, resets **Nov 30 5 PM** — probe in
`mailbox/files/0276/copilot-quota.txt`), so Antigravity substituted, as 0275 did:

| Pass | Model | Result |
|---|---|---|
| 1 | Claude Sonnet 4.6 | 7 findings, `Verdict: FIX` — all fixed |
| 2 | Claude Sonnet 4.6 | 3 new findings, `Verdict: FIX` — all fixed |
| 3 | Gemini 3.8 Flash (Claude pool hit 0%) | all resolved, no new HIGH/MEDIUM, `Verdict: SHIP` |

A Copilot pass over `f67e64b` remains owed when its quota returns.

## Who built it, and what I corrected

Newton built `server.py` through Herdr (`agy -p`, `claude-sonnet-4-6`, SUCCESS in 202 s, 1 turn).
I wrote the stand-in, the proof harness and the README, and corrected four things in Newton's
server: a confirmation digest that could not see an owner change; no stale-state guard (added
`from_digest`); a status value that disagreed with its own `get_game`; and an error message that
did not name the side to move.

## One thing for Juno to settle

0276 names `/pocket-conquest/api/new | /move | /games/<id>.json`; **0279** says the live page
calls `{base}/games`, `{base}/games/{id}/move` and `{base}/games/{id}`. I built to 0276's stated
shape and did not guess. Details and the exact quote: `mailbox/files/0276/api-contract-divergence.txt`.
Only three URL constants and the response field names would move; the engine and the proof stand.
