---
task_id: "0224"
protocol_version: 1.3.0
status: staged
iteration: 1
expect-reply-within: 48h
proof:
  branch: hermes/0224-version-and-example
  sha: 0c50f94328f02ef2fd361dba17121dd8ffa6ce5f
  run: none
  run_note: >-
    Earned by enumeration, not assumed: /home/astroboy/cabin-play has no .github directory at all
    (`ls -d .github` -> "No such file or directory"; `git ls-tree -r --name-only origin/main | grep -c '^\.github/'` -> 0),
    so the repo has no workflows on any branch and a push starts no run. The acceptance evidence is the pytest suite
    (32/32 locally and 32/32 on the VPS), the live endpoint probes quoted below, and the Playwright pass over the deployed app.
  files:
    - writing-peer-review/app/config.py
    - writing-peer-review/app/db.py
    - writing-peer-review/app/example_story.py
    - writing-peer-review/app/main.py
    - writing-peer-review/static/app.js
    - writing-peer-review/static/index.html
    - writing-peer-review/static/styles.css
    - writing-peer-review/static/sw.js
    - writing-peer-review/tests/test_smoke.py
  deployed: "deploy@89.117.21.105:/home/deploy/cabin-play @ 0c50f94, systemd unit writing-review restarted, live at writing.nordtronics.io"
  db_backup: "/home/deploy/backups/cabin-play-20261120T040525Z.db (sqlite .backup, taken before the checkout)"
  screenshots_dir: mailbox/files/0224/
  screenshots:
    - history-1280.png
    - history-380.png
    - settings-1280.png
    - settings-380.png
    - example-story-1280.png
    - example-develop-1280.png
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
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
  notes: >-
    The harness exposes no per-call DeepSeek usage to this cron session, so the token and cost fields are `unknown`
    rather than estimated. One DeepSeek call was made *by the app*, not by this agent: the review text embedded in the
    Example Story fixture was generated once through the app's own provider module on the VPS (provider="deepseek")
    and frozen into `app/example_story.py`, so a fresh database seeds it without a provider call.
notes: |
  STAGED — version display and the protected "Example Story" are built, deployed and proven live. Run 2026-11-19
  21:00-21:10 MST (2026-11-20 04:00-04:10 UTC), off-peak (PEAK: OFF-PEAK 04:00 UTC), PROTOCOL MATCH 1.3.0.

  QUEUE AT PICKUP. active/ held 0142 (superseded), 0159 (decision-blocked), 0187 (superseded), 0209
  (decision-blocked), 0213 (decision-blocked), 0214 (decision-blocked), 0216 (decision-owed) — all left
  completely untouched: no notes edit, no iteration change, no re-investigation. inbox/ held 0176 (superseded),
  0221 (HELD — "Do not pick up") and 0224. 0224 was the only actionable task; claimed with the transition helper
  at e7d9366 (iteration 1).

  STEP 1 — VERSION. One string, one place: `APP_VERSION = "1.1.0"` in `writing-peer-review/app/config.py`.
  `/api/status` and `/api/settings` both report it, and the Settings view carries a quiet footer line reading
  exactly "Version 1.1.0" (`<span id="app-version">`, filled from the API and kept current by the existing
  60 s status poll). Rendered live in a fresh browser profile and measured: text present at 1280 and at 380 px,
  contrast 6.38:1 (rgb(95,85,70) on rgb(246,239,223)). The service-worker cache moved v6 -> v7 in the same
  commit, as the task asked.

  STEP 2 — THE EXAMPLE STORY.
  Mechanism (stated as the task asks): a new column `pieces.is_fixture` (integer, default 0), plus a seeding
  function `ensure_example_story()` in the new module `app/example_story.py`, called from `init_db()` on every
  start. It is a fixture, not a database edit: a fresh database, a restart and a deploy all end with exactly one
  protected Example Story.
    * Adopt, don't duplicate: on the live database the seeding found the existing "0223 E2E" piece (id 8),
      renamed it to "Example Story", set is_fixture = 1, and left its body byte-identical (410 chars, still
      Marta/Elias). It then added the parts the E2E piece was missing — a review (id 5), nothing else, because
      the bible entries and the "Storm chapter" Develop thread were already there.
    * Older databases migrate: if `pieces` predates the flag, `init_db` runs `ALTER TABLE pieces ADD COLUMN
      is_fixture INTEGER NOT NULL DEFAULT 0` before seeding, so no row is touched by the migration itself.
    * Idempotent: content is only filled where it is missing. Proven live with a second restart —
      pieces 4 / fixtures 1 / reviews 4 / bible 4 / threads 2 / messages 4 before and after, unchanged.
  Contents of the seeded showcase (all present on the live piece, verified over HTTP): the Marta/Elias scene
  (78 words), one review carrying all five headings, four story-bible entries (character Marta, character
  Elias, decision "The light stays dark", world "The north storm"), and the "Storm chapter" Develop thread
  with 4 messages. The review text was generated once by the app's real DeepSeek provider for this piece
  (provider="deepseek", 2,528 chars, "## What worked / ## Craft points / ## A question for you /
  ## Follow-up exercise / ## Voice check") and then frozen into the fixture, so the same showcase seeds
  offline. Nothing in its copy uses developer jargon.
  DELETE PROTECTION — server-side, not UI-only. `DELETE /api/pieces/{id}` looks the row up first and returns
  403 `{"detail":"This piece is part of the app and can't be deleted."}` when is_fixture is set; 404 is
  unchanged for a missing id. The History card shows an "Example" badge instead of the Delete button for the
  fixture, and if a stale client still calls the endpoint the alert quotes the server's own sentence.
  Every other piece keeps its Delete button and its 200 response.

  VERIFICATION — WHAT WAS ACTUALLY RUN, IN ORDER.
   * pytest: 32 passed locally at the branch tip (baseline was 27; 5 new tests) and 32 passed on the VPS inside
     the deployed tree. New coverage: the version is reported and rendered (26); the fixture is complete —
     writing, five-heading review, bible entries, Develop thread (27); the delete endpoint refuses the fixture
     with 403, the row survives, and an ordinary piece still deletes with 200 (28); a second `init_db` repairs
     rather than duplicates it (29); a legacy database with no `is_fixture` column and a live "0223 E2E" piece
     is migrated and adopted, keeping its own words and its 2-piece total (30). The two pinned service-worker
     assertions (tests 14 and 24) moved v6 -> v7 with the required bump — declared as a scope extension.
   * Live endpoint probes on the deployed app (127.0.0.1:8765 behind nginx):
       GET /api/status      -> "version":"1.1.0"
       DELETE /api/pieces/8 -> HTTP 403, the sentence above
       POST /api/pieces (throwaway) then DELETE -> HTTP 200
       GET /api/pieces after -> Example Story (fixture) + Bethany's three pieces (not fixtures), unchanged
     The throwaway piece existed only long enough to prove the ordinary delete path on the live app and was
     removed; it left no row behind (piece list identical before and after).
   * Fixture survives deploy and restart: the seeding ran on the deploy's service start (review id 5 created
     then) and a second explicit restart changed no counts (quoted above).
   * UI, Playwright + Chromium against the deployed app through an SSH tunnel (127.0.0.1:18766):
       fresh profile -> data-theme = "warm", localStorage empty (Warm still the default);
       History at 1280 and 380 -> cards are ["Example Story" (no Delete, "Example" badge), "First piece",
       "The unexpected guard", "Second piece" (all with Delete)], no horizontal overflow at either width;
       Settings at both widths -> "Version 1.1.0";
       opening the Example Story -> title, 78-word body, 1 review card badged DEEPSEEK rendering all five
       headings, Develop room showing bible entries, the "Storm chapter" tab and 4 transcript messages,
       no horizontal overflow.
   * Bethany's data: her three pieces and their three reviews are byte-identical to the pre-deploy snapshot
     (ids 2/4/5, titles, updated_at and body lengths equal; reviews 2/3/4 unchanged) — compared programmatically
     against a snapshot taken before the checkout, not by eye. The only new rows are the fixture's review (id 5)
     and the flag on piece 8. Her pieces remain deletable.
   * Public host: https://writing.nordtronics.io/ answers 401 with `WWW-Authenticate: Basic realm="Writing Peer
     Review"` — Juno's existing basic-auth gate is in front, and nginx `proxy_pass`es every path to
     127.0.0.1:8765, which is the revision verified above.

  DEPLOY. Database backed up first (`/home/deploy/backups/cabin-play-20261120T040525Z.db`, sqlite `.backup`,
  taken before any checkout). The VPS remote's fetch refspec is main-only, so the branch was fetched by
  explicit refspec, checked out as `hermes/0224-version-and-example` at 0c50f94 (equal to the branch tip on
  origin), and `writing-review` restarted (ActiveEnterTimestamp 2026-11-20 05:05 CET, NRestarts=0). The gate's
  own log shows the app serving `/api/status` with version 1.1.0 afterwards.

  DEVIATIONS, DECLARED.
   (a) THE COPILOT PEER-REVIEW GATE DID NOT RUN. herdr-core pane t17 ("copilot-main", live, idle) still answers
       "Please use /login to sign in to use Copilot" (read from the pane this run). I did not substitute another
       reviewer and did not fabricate a review; the substitute is Juno's own verification of this reply. The fix
       is `/login` in pane t17 and one pass over the branch diff.
   (b) NO PR AND NO MERGE, same as 0223: the fine-grained PAT in this profile is contents-only and the gh keyring
       login is invalid, so the branch is pushed but not opened as a PR. Main still carries the pre-0224 app; the
       live site runs the branch revision.
   (c) tests 14 and 24 pinned the old cache version and moved with the required bump (scope extension, the
       criterion being the bump itself). No other pre-existing test changed.
   (d) The throwaway piece created on the live app for the ordinary-delete probe was deleted in the same probe;
       the piece list is identical before and after and its id (11) is simply skipped by SQLite's sequence.
   (e) The fixture's review text was produced by one real provider call during this run and frozen into
       `app/example_story.py`; it is fixture content from that point on, not a live call at seed time.

  NOT DONE IN THIS TASK: nothing in the spec is left unbuilt. The Example Story stays editable (only deletion is
  refused) — the task asked for permanence, not read-only — and it can be renamed only by editing the fixture,
  because every start restores its title.
---

# 0224 executed — version 1.1.0 on screen, and the Example Story is a permanent fixture

**Status: shipped and deployed.** Branch `hermes/0224-version-and-example` @ `0c50f94`, 32/32 tests locally and
on the VPS, live at writing.nordtronics.io behind Juno's basic-auth gate. No CI run exists in this repo (no
workflows) and no PR was opened (no working GitHub write credential in this profile) — both declared above.

## The version

`1.1.0` — defined once, in `writing-peer-review/app/config.py` (`APP_VERSION`), reported by `/api/status` and
`/api/settings`, and shown at the foot of **Settings** as "Version 1.1.0". It is visible at 1280 px and at
380 px without scrolling past anything else, so Stephen, Bethany and Juno can all quote the same number.
The service-worker cache moved to `writing-peer-review-v7` in the same commit, so Bethany's installed app picks
the new shell up on next open.

## The Example Story

- **Title:** exactly "Example Story", listed at the top of the archive.
- **Can't be deleted, and not just in the UI:** `DELETE /api/pieces/8` answers
  `403 {"detail":"This piece is part of the app and can't be deleted."}`. In History it shows an "Example" badge
  where the Delete button would be. Everything else about it stays usable — open it, edit it, review it, talk to
  the Develop partner.
- **How permanence works:** `pieces.is_fixture` plus a seeding function that runs on every start. If the piece is
  missing (a new database), it is built; if it is present, it is repaired, never duplicated; on the live
  database it adopted the old "0223 E2E" project rather than making a copy. A restart was run twice to prove the
  counts do not move.
- **What is inside it:** the Marta/Elias scene, a full peer review (What worked / Craft points / A question for
  you / Follow-up exercise / Voice check, band "Reads human"), four story-bible entries — Marta, Elias, the
  decision that the light stays dark, and the north storm — and the "Storm chapter" Develop conversation with
  two exchanges, so every feature has something real to look at.

## Bethany's own work

Untouched, and still deletable. Her three pieces and their three reviews compare byte-identical to the snapshot
taken before the deploy (`Second piece`, `The unexpected guard`, `First piece`). Nothing about this build reads
or rewrites them; the only writes to the database were the fixture's flag, its title, and one review row.

## Evidence

| Criterion | Evidence |
|---|---|
| Version shown, matches the reply | `/api/status` -> `"version":"1.1.0"`; Settings reads "Version 1.1.0" at 1280 and 380 (screenshots) |
| "Example Story" under exactly that title | History card title on the live app, both widths |
| Delete absent in UI and refused by the endpoint | no `.piece-delete-btn` on its card; `DELETE /api/pieces/8` -> HTTP 403 |
| Showcase contents present | 1 review (5 headings), 4 bible entries, 1 Develop thread with 4 messages, all read back over HTTP |
| Three real pieces untouched and deletable | pre/post snapshot equality; ordinary-piece delete on the live app -> HTTP 200 |
| pytest passes, plus delete-protection coverage | 32/32 local, 32/32 VPS; 5 new tests (26–30) |
| Survives restarts/deploys | second restart: identical row counts; seeding is idempotent by design and test |

Screenshots are in `../files/0224/`: `history-1280.png`, `history-380.png`, `settings-1280.png`,
`settings-380.png`, `example-story-1280.png`, `example-develop-1280.png`.
