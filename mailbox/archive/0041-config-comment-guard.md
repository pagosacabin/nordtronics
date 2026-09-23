---
task_id: "0041"
status: staged
iteration: 1
proof:
  - branch: hermes/0041-config-comment-guard
    sha: 6fee46a960fe786c087091de3c91bc6c9cc7c31f
notes: |
  Status: done.

  Mechanism (one paragraph): `hermes config set` cannot preserve comments — it re-dumps the
  config with yaml.safe_dump, which drops every comment line AND every trailing comment. So
  instead of trying to change the writer, a guard keeps a *ledger* of comments outside the
  config (~/.hermes/config-comments/ledger-<slug>.yaml), keyed by the config key each comment
  annotates, and re-injects what a writer dropped: whole comment runs at their anchor key,
  trailing comments back onto their line with the original column padding. It learns new
  comments automatically (anything you hand-write is captured on the next run), and it is
  triggered three ways so no writer has to cooperate: a systemd user path unit fires it
  seconds after any change to a watched config, a 10-minute timer is the backstop, and
  ~/.hermes/bin/hermes-config-guarded is a wrapper (capture -> run hermes -> restore) for
  writers that want the comments back in the same call.

  Before/after: (a) a bare `hermes config set` on a byte-for-byte copy of the live config
  took it from 4 comments to 0; the same write on the live config also read 0 comments
  *immediately* after the write and 4 again four seconds later, restored by the path unit
  (systemd: Result=success, ExecMainStatus=0) — no wrapper, no cooperation. (b) The cron
  worker's own profile config went from 20 whole-line + 18 trailing comments to 0 + 0 and back
  to 20 + 18. Non-comment content is byte-identical across every restore, and the parsed value
  tree is unchanged (yaml.safe_load equality), so no config value moved. The live config ends
  byte-identical to its pre-task bytes (sha256 b941422f0acfa8e7630a2a90057095bee129662255364e7adb0aef9d247778de,
  same as the backup taken before any change).

  Also in place: a 14-check self-test (0 failing), a guard log, pre-change backups of every
  file it rewrites (5 kept), an flock so the three triggers serialise, and a handoff-mailbox
  skill note telling cron runs to write config through the wrapper.

  No CI run exists for this branch: android-build.yml is pinned to branch
  android-toolchain-setup and platformio.yml is path-filtered to firmware/**, so nothing
  matches ops/** — proof is the branch + tip SHA only, deliberately with no run pointer.

  Deliberately out of scope: the `tim` profile's config is another agent's file and is not
  guarded (one line in ~/.hermes/config-comments/targets.conf plus a PathChanged= adds it).

  Branch: hermes/0041-config-comment-guard @ 6fee46a960fe786c087091de3c91bc6c9cc7c31f
  (ops/config-comment-guard/: guard, wrapper, systemd units, install.sh, README, evidence).
---

# 0041 — Stop automated config writes from stripping explanatory comments

## Context
Your 2026-09-22 writeup correctly scoped the hazard: `~/.hermes/config.yaml` has up to three writers (your interactive session, the 15-minute cron worker, possibly the tim profile), and config writes strip comments — including the ones explaining the FreeCAD ABI pin to the snap's Python 3.12 and the Flatpak TMPDIR rationale. The silent-rot risk is real: six weeks from now someone debugs FreeCAD and starts from nothing.

## Task
Implement a guard so automated config writes preserve the explanatory comment blocks. Your choice of mechanism — your own suggestion (diff against the newest backup after any config write, then restore the comment block) or adding preservation guidance to the handoff-mailbox skill so the cron worker keeps them. Pick the one you would actually trust unattended.

## Success criteria
1. A config write of the kind the cron worker performs no longer strips the explanatory comments.
2. Demonstrated before/after: show the comment block present after a write that previously would have removed it.
3. The config remains valid and loads (your usual MCP connectivity check still passes).

## Constraints
- Back up the config before changing anything.
- Do not alter any actual config values — comments only.
- This is your machine and your config; touch no one else's files.

## Proof
- Branch `hermes/0041-config-comment-guard` pushed to origin with whatever implements the guard (script, skill edit, wrapper — your call).
- Staged reply with the branch + tip SHA, the mechanism in one paragraph, and the before/after evidence.

## Reply format
Status + branch/SHA + mechanism + before/after evidence, per the canonical staged-reply format.

---

## Reply

### 1. Status

Done. Guard implemented, installed, enabled, and exercised against the real `hermes config set`
on both function-list configs.

- Branch `hermes/0041-config-comment-guard`, tip `6fee46a960fe786c087091de3c91bc6c9cc7c31f`
- Files: `ops/config-comment-guard/{hermes-config-guard.py, hermes-config-guarded,
  hermes-config-guard.{service,path,timer}, install.sh, targets.conf.example, README.md,
  evidence/verify-before-after.sh, evidence/0041-before-after.txt}`

### 2. Mechanism (one paragraph)

`hermes config set` (and `unset`, `mcp add/remove`) reads the user config, mutates the tree and
re-dumps it via `hermes_cli/config.py::_write_user_config` → `atomic_yaml_write` →
`yaml.safe_dump`. Comments are not in the parsed tree, so **every** write drops **every** comment
— whole-line runs and trailing end-of-line comments alike. Rather than fight the writer, the guard
keeps a *ledger* of the comments outside the config
(`~/.hermes/config-comments/ledger-<slug>.yaml`), keyed by the config key each comment annotates
(dotted path + enclosing section path, never line numbers, so it survives reordering), and
re-injects what a writer dropped: whole runs inserted at their anchor key re-indented to the
anchor's current column, trailing comments re-appended with the original spacing. It **learns**
automatically (any comment present in the file but absent from the ledger is captured, so a
hand-written note is defended from the next write onward) and needs no cooperation from the
writer: a systemd user **path unit** fires it seconds after any change to a watched config, a
**10-minute timer** is the backstop for a write that lands mid-run, and
`~/.hermes/bin/hermes-config-guarded` is a drop-in **wrapper** (capture → run `hermes ...` →
restore) for callers that want the comments back in the same call — that wrapper is what the
cron worker should use, and the handoff-mailbox skill now says so.

Restores are comment-only and fail closed: the guard refuses to write (exit 2, nothing changed)
unless the file with every comment removed is byte-identical before and after *and* the parsed
value tree is unchanged; comment lines inside block scalars (`notes: |`) are never touched; a
missing anchor, an ambiguous match or an unparseable ledger is skipped and logged rather than
guessed at; every rewrite is preceded by a backup in `~/.hermes/backups/config/` (5 kept) and an
flock serialises the three triggers.

### 3. Before/after evidence

Full transcript: `ops/config-comment-guard/evidence/0041-before-after.txt` (reproducible with
`evidence/verify-before-after.sh`). Excerpts:

**Before — the writer, on a byte-for-byte copy of the live config:**

```
=== 1. BEFORE — a bare 'hermes config set' deletes every comment
   comment lines before: 4
     262:      # Flatpak KiCad has a private /tmp+/var/tmp; scratch copies must live under $HOME
     270:    # FreeCAD Robust MCP server (spkane/freecad-addon-robust-mcp-server), embedded
     271:    # headless mode. Launcher runs the server under the FreeCAD snap's own Python
     272:    # 3.12 because FreeCAD.so is ABI-bound to 3.12 (host pythons are 3.11/3.14).
   comment lines after : 0
```

**After — the same bare write on the live config, units enabled, no wrapper, no cooperation:**

```
=== 3. LIVE — same bare write on ~/.hermes/config.yaml, no wrapper, units enabled
   pre-write: 4 whole-line + 0 trailing comment lines
   immediately after the write (nothing has had time to intervene): 0 whole-line
   four seconds later: 4 whole-line + 0 trailing
     262:      # Flatpak KiCad has a private /tmp+/var/tmp; scratch copies must live under $HOME
     270:    # FreeCAD Robust MCP server (spkane/freecad-addon-robust-mcp-server), embedded
     271:    # headless mode. Launcher runs the server under the FreeCAD snap's own Python
     272:    # 3.12 because FreeCAD.so is ABI-bound to 3.12 (host pythons are 3.11/3.14).
   systemd: who fired the restore
     Result=success
     ExecMainStartTimestamp=Tue 2026-09-22 21:55:34 MDT
     ExecMainStatus=0
     active: active          # hermes-config-guard.path
     active: active          # hermes-config-guard.timer
   guard log (tail)
     ... restored [/home/astroboy/.hermes/config.yaml] 2 block(s) / 4 comment line(s) + 0 trailing ...
```

**After — the cron worker's own profile config, trailing comments included:**

```
=== 6. The cron worker's own profile config: whole-line AND trailing comments survive a real write
   pre-write: 20 whole-line + 18 trailing comments
   immediately after the write: 0 whole-line + 0 trailing
   four seconds later: 20 whole-line + 18 trailing
     28:    - a2a            # agent-to-agent chat surface — interactive only
     29:    - browser        # browser-use cloud session; git/gh/KiCad need no browser
     ...
     221:  # Cron runs resolve their toolset from THIS key
   semantic check — value tree identical to the pre-write bytes:
     values identical
```

**The guard's own tests** (`hermes-config-guard.py --selftest`): 14 checks, `0 failing` — learn
is a no-op on a comment-bearing file, restore re-adds whole-line runs and trailing comments with
original padding exactly once, second run is a no-op, comment-looking lines inside a block scalar
are untouched, anchors resolve after a reordering writer, `--check` exits 1 on a stripped file
and writes nothing, invalid YAML and stale/ambiguous anchors are refused or skipped. The final
end-to-end check reads `python3 ~/.hermes/bin/hermes-config-guard.py --all --check` → exit 0,
"all 2 ledger block(s) present" / "all 25 ledger block(s) present".

### 4. Success criteria

1. **No longer strips the comments** — a real `hermes config set` on both configs now ends with
   all comments present (sections 3 and 6 above), restored automatically within seconds.
2. **Before/after shown** — 4 → 0 → 4 whole-line comments on the live config; 20 + 18 → 0 → 20 +
   18 on the cronrunner config; systemd service `Result=success` at the restore timestamp.
3. **Config remains valid and loads** — `hermes config check` reports "Config version: 42 ✓";
   `hermes mcp list` shows both servers `✓ enabled`; `hermes mcp test kicad` connects and returns
   the 233-tool schema.

### 5. Constraints honoured

- **Backed up first**: `~/.hermes/config.yaml.bak-0041-20260922-195255` and
  `~/.hermes/profiles/cronrunner/config.yaml.bak-0041-20260922-195255` were taken before any
  write, and both configs are left byte-identical to them (verified by sha256, section 7 of the
  transcript: `default` and `cronrunner` "sha256 now" = "sha256 pristine").
- **Comments only, no value changed**: every restore is checked to leave the non-comment content
  byte-identical and the parsed value tree equal; the demonstration writes set an existing key to
  its existing value. The only byte-level difference the *writer* itself introduces is YAML
  re-quoting of one existing string (`"120000"` → `'120000'`, same parsed value); the guard does
  not do this and the file was restored to its original bytes afterwards.
- **Nobody else's files**: only `~/.hermes/config.yaml` and the cronrunner profile's own config
  are watched; the `tim` profile config is untouched (and is one line away from being added).

### 6. Verification you can run

```bash
python3 ~/.hermes/bin/hermes-config-guard.py --all --check      # exit 0 = every comment in place
python3 ~/.hermes/bin/hermes-config-guard.py --selftest          # 14 checks
python3 ~/.hermes/bin/hermes-config-guard.py --all --dry-run     # what a restore would insert
~/.hermes/bin/hermes-config-guarded config set <key> <value>     # write with comments preserved
systemctl --user status hermes-config-guard.path hermes-config-guard.timer
tail -20 ~/.hermes/config-comments/guard.log
```

### 7. Known limits (leftovers, not failures)

- A comment is restored only where its anchor still exists. If a future writer *renames* the key
  a note hangs off, the note stays in the ledger and is logged as a skip; the guard never moves a
  comment speculatively.
- A trailing comment whose text now appears on lines in two different sections is restored only
  when the enclosing section still matches; otherwise it is skipped and logged (this is the bug
  the self-test's "ambiguous item" case pins down).
- Re-seeding:`install.sh` re-learns from the live files and is idempotent; ledgers are per config
  path in `~/.hermes/config-comments/`.
