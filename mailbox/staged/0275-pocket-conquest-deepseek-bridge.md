---
task_id: "0275"
title: "Pocket Conquest rules engine + DeepSeek bridge (interim opponent)"
status: staged
iteration: 2
expect-reply-within: 24h
proof:
  branch: hermes/0275-pocket-conquest
  repo: pagosacabin/cabin-play
  sha: 08efcd2802482886b60dcc854a221a32ae6df9ef
  run: none
  run_note: "cabin-play carries no `.github/` at all: `git ls-tree -r --name-only 08efcd2 | grep -i '.github'` is empty and the checkout has no .github/workflows directory, so no workflow could trigger on this branch. No ntfy receipt either: there is no compiled artifact (a Python service on the host, committed to cabin-play), and the task names no topic."
  copilot_review: >-
    NOT AVAILABLE -- no Copilot verdict exists. Pane t17 (copilot-main) answers
    "You have exceeded your monthly quota (Request ID: 00000-c928d8c3-4fce-4d5f-8036-13995e3ba537)"
    at turn 342 / Session 138.23 AIC used (mailbox/files/0275/copilot-quota.txt);
    that is a live read of the pane today, not a memory of the earlier failure.
    SUBSTITUTE, declared as a deviation: Antigravity (Claude Sonnet 4.6), 5 passes,
    all transcripts committed -- pass 1 (7-item security checklist, all PASS, plus
    findings), pass 2, pass 3 (SHIP, one open LOW), pass 4 (SHIP after the LOW-1
    fix, found LOW-2), pass 5 on the final server.py (no findings, SHIP). Both open
    findings were fixed in code, not argued.
  serve_url: "https://home.tail2bff3f.ts.net/pocket-conquest  -- BLOCKED PENDING STEPHEN: publishing it needs root, see notes."
  files:
    - pocket-conquest-bridge/pocket_conquest.py
    - pocket-conquest-bridge/agent_core.py
    - pocket-conquest-bridge/server.py
    - pocket-conquest-bridge/hardening.py
    - pocket-conquest-bridge/proof.py
    - pocket-conquest-bridge/run.sh
    - pocket-conquest-bridge/requirements.txt
    - pocket-conquest-bridge/README.md
    - pocket-conquest-bridge/pocket-conquest-bridge.service
    - pocket-conquest-bridge/.gitignore
  applied_to:
    - /home/astroboy/.local/share/pocket-conquest-bridge/app/ (deployed copy; server.py sha256 17f7fa1bee0eef17... identical to 08efcd2's blob)
    - /home/astroboy/.config/systemd/user/pocket-conquest-bridge.service (user unit, enabled)
  evidence:
    - mailbox/files/0275/curl-proof.txt
    - mailbox/files/0275/local-proof.txt
    - mailbox/files/0275/hardening.txt
    - mailbox/files/0275/path-shapes.txt
    - mailbox/files/0275/options-403-probe.txt
    - mailbox/files/0275/token-usage.txt
    - mailbox/files/0275/serve-blocked.txt
    - mailbox/files/0275/copilot-quota.txt
    - mailbox/files/0275/review-pass1-antigravity-claude-sonnet-4-6.txt
    - mailbox/files/0275/review-pass2-antigravity-claude-sonnet-4-6.txt
    - mailbox/files/0275/review-pass3-antigravity-claude-sonnet-4-6.txt
    - mailbox/files/0275/review-pass4-antigravity-claude-sonnet-4-6.txt
    - mailbox/files/0275/review-pass5-antigravity-claude-sonnet-4-6.txt
    - mailbox/files/0275/review-pass4-prompt.txt
    - mailbox/files/0275/options-403-probe.py
    - mailbox/files/0275/ratelimit-probe.py
cost:
  model: deepseek-flash
  api_calls: 28
  input_tokens: 15633
  input_cache_hit_tokens: 8832
  input_cache_miss_tokens: 6801
  output_tokens: 124
  total_tokens: 15757
  search_requests: 0
  rate_window: off-peak
  rates_used: "$0.15/M input miss, $0.003/M input hit, $0.60/M output (off-peak DeepSeek)"
  cost_usd: 0.001121
  cost_note: >-
    api_calls/tokens are the bridge's OWN DeepSeek calls logged by the service between
    14:00 and 14:20 MST today (28 calls: the proof suite, the curl proof, the path-shape
    probe and the probes' model turns), summed from the usage objects in
    mailbox/files/0275/token-usage.txt. The agent's own per-call usage is not visible to
    this harness, so the worker-side token figures are not estimated here.
notes: |
  DELIVERED, with two things still owed to Stephen: the Serve mount (root) and a
  Copilot verdict (monthly quota). Neither is invented below.

  WHAT SHIPPED. cabin-play:pocket-conquest-bridge/ on branch hermes/0275-pocket-conquest,
  tip 08efcd2. A Python 3.11+ rules engine plus a tiny HTTP service, stdlib only, no
  dependency installed:
    pocket_conquest.py  legal-move generation, move validation, combat, income, turn
                        handoff, --selftest (11 checks).
    agent_core.py       the prompt builder (condensed rules + board + gold + armies +
                        numbered legal list + a TACTICS line), the reply parser, and a
                        retrying pick_legal that re-validates every answer.
    server.py           HTTP/1.1 service: exact-path allowlist, CORS, token bucket,
                        worker-slot semaphore, one deadline for the whole request,
                        header/body caps, drain+close on every rejection, JSON journal.
    hardening.py        11 hostile-shape checks (raw sockets).
    proof.py            end-to-end proof: real positions, every returned move re-checked.
    run.sh / .service / README.md / requirements.txt

  MOVE GRAMMAR (also in README):  "B <tile> <n>" build n at a capital/owned town;
  "M <src> <dst> <n>" move or attack with n; "M <src> <dst> <n> + <src2> <n2>" pincer
  with a second friendly stack; "END" collect income and hand over. Unambiguous by
  construction -- every field is named, and the model may only answer with one entry
  from the list the page sent (validated by the engine, retried once with the error
  appended, then HTTP 500 so the page falls back).

  RULES FIDELITY. The task's rules are implemented as written, including the three
  easy-to-miss ones the selftest pins: only armies raised THIS turn are immobile
  (built_armies, not a tile-wide flag), a town captured this turn cannot be built at
  (captured_this_turn), and a pincer's support stack stays put while the main stack
  advances. One reading is recorded here because the text leaves it open: END is
  always legal, so a side is never forced to move -- that is how the engine ends a
  turn and it is what the page's move list carries.

  MODEL COST PER MOVE (deepseek-flash, thinking disabled, measured on the final build,
  17 logged calls today): 384-756 prompt tokens (0-512 of them cache hits) and 1-9
  completion tokens, 0.58-0.81 s round trip. Well inside the page's 30 s and the 6 s
  client timeout. Token lines: mailbox/files/0275/token-usage.txt. The README's earlier
  "1-7 completion" line was wrong (one case spends 9) and is corrected in 51da435.

  PROOF, run today against the deployed service on 127.0.0.1:8767:
    - curl (literal, mailbox/files/0275/curl-proof.txt):
        POST /pocket-conquest-move  ->  {"move": "M B5 B4 1"}  HTTP 200  0.60 s
        GET  /health                ->  {"ok": true}            HTTP 200
    - proof.py, four positions incl. the one where build/move/attack are all legal
      and one where Emberhold is takeable (mailbox/files/0275/local-proof.txt):
        initial           -> M B5 B4 1    0.65 s  legal, from 12 options (2 build/9 move/1 END)
        R to move         -> M E1 D1 3    0.58 s  legal, from 12 options (2 build/6 move/3 attack/1 END)
        midgame           -> M C5 C4 5    0.58 s  legal, from 34 options (6/22/5/1)
        siege             -> M E2 E1 1    0.73 s  legal, from 35 options (1/27/6/1)
        garbage body / missing position -> 400 fail-closed, 0 failing cases
    - hardening.py against the same instance: 11/11 PASS (garbage JSON, no position,
      unknown path, unknown method, oversized Content-Length, chunked TE, 16 KiB header
      block, pipelined-after-rejection, pipelined-behind-404, /health, OPTIONS).
      The rate limiter is checked separately on a scratch instance (burst=3, 6/min):
      [400, 400, 400, 429, 429] -- mailbox/files/0275/hardening.txt.

  SERVE -- BLOCKED PENDING STEPHEN, exactly as the task allows. Serve config on this
  host belongs to root; `sudo -n` answers "sudo: a password is required" and
  `tailscale serve status` shows only `/` and `/chese-move`, so `/pocket-conquest` is
  not mounted (mailbox/files/0275/serve-blocked.txt). The one command is:

      sudo tailscale serve --bg --https=443 --set-path /pocket-conquest http://127.0.0.1:8767

  and the URL he pastes into the page's agent box is

      https://home.tail2bff3f.ts.net/pocket-conquest

  One correction found while preparing that: `tailscale serve --set-path` KEEPS the
  mount prefix in the forwarded path (proven on the chess bridge, where the upstream
  sees /chese-move/chess-move), so a URL-shaped POST would have arrived at
  /pocket-conquest/pocket-conquest-move and 404'd. The service now accepts the mount
  root, the trailing-slash form, the prefixed move path and the bare localhost path
  (exact strings only, no prefix matching), and health at /health and
  /pocket-conquest/health. Proved locally in mailbox/files/0275/path-shapes.txt:
  5 accepted shapes -> 200 with a legal move, 2 lookalikes -> 404, GET /pocket-conquest
  -> 404. This is the f0ea732 change, reviewed in pass 5 (no findings, SHIP).

  START / STOP (exact, on this host):
      systemctl --user start|stop|restart pocket-conquest-bridge.service
      journalctl --user -u pocket-conquest-bridge.service -f
      ~/.local/share/pocket-conquest-bridge/app/run.sh        # foreground instead
  The unit is enabled and runs as a user unit with NoNewPrivileges, empty capability
  set, PrivateTmp/PrivateDevices, ProtectSystem=strict and ProtectHome=read-only. The
  DeepSeek key is never in the repo: run.sh reads DEEPSEEK_API_KEY from ~/.hermes/.env
  at start (git grep over the branch finds only the variable name, no value).

  REBOOT SURVIVAL -- mechanism verified, reboot NOT performed. `Linger=yes` for
  astroboy, so user@1000.service is started by PID 1 at boot; it has been up since
  this boot (2026-11-18 13:36:53) while the graphical login is a later, separate
  session, and the sibling chess-bridge starts the same way. So no manual restart
  should be needed. I did not reboot: the reboot belongs to 0262/0264 and the harness
  hard-blocks it -- and the README's earlier claim ("does not come back on its own at
  boot") was wrong for this host; 08efcd2 corrects it to the verified mechanism.

  REVIEW -- what actually happened. Copilot could not review: pane t17 reports the
  monthly quota exhausted (verbatim message + Request ID in evidence). Rather than
  stage a task whose named gate silently did not run, the build went through Antigravity
  (Claude Sonnet 4.6) five times and the transcripts are committed:
    pass 1  7-item checklist all PASS (no hardcoded secrets / inputs bounds-checked /
            auth on every endpoint / TLS off-box / least privilege / fail-closed errors /
            pinned deps -- the last one is "stdlib only", so there is nothing to pin)
            plus findings: HIGH (request smuggling via undrained bodies on reject paths),
            MEDIUM (429 left the body unread), and smaller items.
    pass 2  accepted the fixes, found HIGH-3/HIGH-4 (404/403 and the secondary 431 still
            did not close) and MEDIUM-5 (the no-legal-move 500).
    pass 3  all four fixed and verified; one new LOW-1 (OPTIONS 403 did not close) ->
            Verdict: SHIP at 125b3f0.
    pass 4  LOW-1 confirmed fixed, new LOW-2 (that branch closed but did not drain) ->
            Verdict: SHIP; LOW-2 fixed in 51da435.
    pass 5  the path-shape change (f0ea732): exact-string membership only, GET still
            health-only, 404 still closes+drains, limiter/slots path-agnostic ->
            "No findings", Verdict: SHIP.
  LOW-2's fix is verified by a discriminating probe, not by the reviewer's word
  (mailbox/files/0275/options-403-probe.txt): on a scratch instance with an origin
  allowlist, OPTIONS with a disallowed origin answers 403 and the socket is closed
  (a second request on it is never answered), while OPTIONS with an allowed origin
  answers 204 and the connection stays alive for the second request -- 3/3 identical
  runs. That probe had a defect on its first version (it did not consume the response
  body, so a trailing body segment read as a second answer and printed a false FAIL);
  the fixed version and both outcomes are in the evidence, because a probe that can
  only pass is not evidence.

  WHO BUILT IT. The route was Newton via Herdr as instructed (pure software), but the
  agy build session returned status ERROR after ~6 minutes; per the rule, I read its
  diff instead of rebuilding -- the draft was there and wrong in the places that matter
  for a game: I rewrote the combat/advance/pincer/income paths against the rules text,
  wrote the prompt builder and the reply parser, wrote the service and the hardening
  checks, and added both review-driven hardening rounds. Newton's contribution was the
  first skeleton; every number in the reply is from code I ran.

  SCOPE EXTENSIONS, declared, each with its reason:
    - proof.py (90a2851): its "R to move" case was mislabelled -- it never applied END,
      so it was still Stephen's turn with both stacks spent (1 legal option). It now
      hands the turn over and exercises R's real 12-option list.
    - README (51da435): measured token range corrected (see above).
    - server.py paths (f0ea732) and README (08efcd2): the mount-prefix handling and the
      reboot-mechanism statement, both explained above. The reboot sentence is a
      correction of an earlier claim in this same branch, not a new claim about a
      reboot I ran.

  CORRECTION DECLARED. The previous tick built all of this and died at ~13:36 MST
  without staging; this run resumed it (iteration unchanged at 2 -- a resume, not a
  re-pickup), re-verified every artifact itself, and added the four commits above.

  DECISION OWED (one): run the sudo tailscale serve command and paste
  https://home.tail2bff3f.ts.net/pocket-conquest into the page's agent box. When
  Copilot's quota resets, a locked-lane pass over tips 51da435..08efcd2 can close the
  last named criterion; nothing in those commits is unverified, they are only
  unreviewed by Copilot.
---

# 0275 — Pocket Conquest rules engine + DeepSeek bridge: delivered

**Status: DONE on the code and the local proof. Two gates are open, both stated plainly: the Serve mount (needs root) and a Copilot verdict (monthly quota).**

## What it is

`cabin-play:pocket-conquest-bridge/` on `hermes/0275-pocket-conquest` (tip `08efcd2`):

| file | what it does |
|---|---|
| `pocket_conquest.py` | the rules engine — legal-move generation, `is_legal`, `apply_move` (combat, pincer, income, turn handoff), `normalise_position`, `--selftest` |
| `agent_core.py` | the prompt (condensed-but-complete rules + board + gold + armies + numbered legal list + `TACTICS:`), the reply parser, retrying `pick_legal` that re-validates |
| `server.py` | the HTTP service: path allowlist, CORS, token bucket, worker slots, one deadline for the request, header/body caps, drain+close on every rejection, JSON journal on stderr |
| `hardening.py` | 11 raw-socket hostile-shape checks |
| `proof.py` | end-to-end proof against a running instance, every move re-validated by the engine |
| `run.sh`, `pocket-conquest-bridge.service`, `README.md`, `requirements.txt` | foreground start, user unit, docs, "no dependencies" |

Rules are implemented as the task states them — the three that are easy to get wrong are pinned by the selftest: only armies raised *this* turn are immobile, a town captured this turn cannot be built at, a pincer's support stack stays put while the main stack advances.

## The Serve URL — blocked pending you

`sudo -n` has no password here, so I cannot mount it. The command:

```bash
sudo tailscale serve --bg --https=443 --set-path /pocket-conquest http://127.0.0.1:8767
```

The URL to paste into the page's **agent box**:

```
https://home.tail2bff3f.ts.net/pocket-conquest
```

`tailscale serve` currently lists only `/` and `/chese-move`, so nothing is published yet. One thing I found while preparing that command: `--set-path` keeps the prefix in the forwarded path (the chess bridge proves it — its upstream sees `/chese-move/chess-move`), so the service now accepts the mount root, `/pocket-conquest/`, `/pocket-conquest/pocket-conquest-move` and the bare localhost path, with lookalikes still 404 (`mailbox/files/0275/path-shapes.txt`).

## Start / stop

```bash
systemctl --user start|stop|restart pocket-conquest-bridge.service
journalctl --user -u pocket-conquest-bridge.service -f
~/.local/share/pocket-conquest-bridge/app/run.sh     # foreground
```

The key is read from `~/.hermes/.env` at start by `run.sh`; `git grep` over the branch finds only the variable name.

## Proof (today, final build)

```
$ curl -sS -X POST http://127.0.0.1:8767/pocket-conquest-move -H 'Content-Type: application/json' \
       --data-binary @0275-curl-body.json -w '\nHTTP %{http_code} total %{time_total}s\n'
{"move": "M B5 B4 1"}
HTTP 200  total 0.596s
$ curl -sS http://127.0.0.1:8767/health
{"ok": true}  HTTP 200
```

```
initial position      -> M B5 B4 1  0.65s  legal (12 options: 2 build, 9 move, 1 end turn)
R to move             -> M E1 D1 3  0.58s  legal (12 options: 2 build, 6 move, 3 attack, 1 end)
midgame               -> M C5 C4 5  0.58s  legal (34 options: 6 build, 22 move, 5 attack, 1 end)
siege (win available) -> M E2 E1 1  0.73s  legal (35 options: 1 build, 27 move, 6 attack, 1 end)
garbage / no position -> HTTP 400 fail-closed                  0 failing cases
```

`hardening.py`: 11/11 PASS. Rate limiter on a scratch instance (burst 3, 6/min): `[400, 400, 400, 429, 429]`.

## Tokens per move

`deepseek-flash`, `thinking: disabled`: **384–756 prompt** (0–512 cache hits) and **1–9 completion** tokens, **0.58–0.81 s** round trip over 17 logged calls.

## Review

Copilot could not run — pane t17 answers *"You have exceeded your monthly quota"* (Request ID in `mailbox/files/0275/copilot-quota.txt`). So there is **no Copilot verdict**, and I am not writing one. The build was reviewed five times by Antigravity (Claude Sonnet 4.6) instead; the transcripts are committed. Pass 1 answered the 7-item checklist (all PASS) and found a request-smuggling HIGH; passes 2–4 fixed and verified four more findings (all fixed in code); pass 5 reviewed the path change and returned **SHIP, no findings**. Both open findings' fixes are additionally proved by a discriminating probe (`options-403-probe.txt`): a refused preflight closes its socket, an allowed one stays alive.

## Reboot survival

Mechanism verified, reboot not performed (it belongs to 0262/0264 and the harness blocks it): `Linger=yes`, so `user@1000.service` starts at boot and has been up since 2026-11-18 13:36:53, while the graphical login is a later session. So the service should come back on its own — and the README's earlier claim that it would not was wrong; `08efcd2` corrects it.

## Decision owed

1. Run the `sudo tailscale serve …` command above and paste the URL into the page's agent box.
2. When the Copilot quota resets, a locked-lane pass over `51da435..08efcd2` closes the last named criterion. Nothing in those commits is unverified — it is only unreviewed by Copilot.
