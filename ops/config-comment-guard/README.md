# Config comment guard (mailbox task 0041)

Automated writers of `~/.hermes/config.yaml` delete every comment in the file.

`hermes config set` / `unset` / `mcp add` read the user config, mutate the tree and re-dump it
(`hermes_cli/config.py::set_config_value` → `_write_user_config` → `atomic_yaml_write`, i.e.
`yaml.safe_dump`). Comments are not part of the parsed tree, so **every** write drops **every**
comment line — including the ones that explain why a value is what it is:

```
      # Flatpak KiCad has a private /tmp+/var/tmp; scratch copies must live under $HOME
      TMPDIR: /home/astroboy/.cache/kicad-mcp/tmp
    # FreeCAD Robust MCP server (spkane/freecad-addon-robust-mcp-server), embedded
    # headless mode. Launcher runs the server under the FreeCAD snap's own Python
    # 3.12 because FreeCAD.so is ABI-bound to 3.12 (host pythons are 3.11/3.14).
    command: /home/astroboy/.local/bin/freecad-mcp-robust
```

Six weeks later, whoever debugs the FreeCAD ABI pin starts from nothing. This guard stops that.

## Mechanism

A **comment ledger** lives outside the config, at `~/.hermes/config-comments/ledger-<slug>.yaml`.
Each entry is either one maximal run of comment lines, or one trailing (end-of-line) comment, plus
the config **key** it annotates, so the comment can be re-found after any rewrite:

```yaml
blocks:
  - kind: comment_block
    anchor: mcp_servers.freecad.command      # dotted key path; re-indented to the anchor's column
    position: before                         # before | after | eof
    lines:
      - '    # FreeCAD Robust MCP server (spkane/freecad-addon-robust-mcp-server), embedded'
  - kind: inline_comment
    anchor: agent.disabled_toolsets
    path: agent.disabled_toolsets            # enclosing key path, so `- a2a` in two lists is
    line: '- a2a'                            # told apart from `- a2a` in the other one
    pad: 12                                  # column padding, so the restored bytes match
    trailing: '# agent-to-agent chat surface — interactive only'
```

Two operations, both keyed off the key path — never off line numbers:

- **learn** — every comment run and trailing comment in the live config that is not already in the
  ledger is added (positions resolved by scanning indentation, list-item mappings and block
  scalars). So a comment you hand-write today is defended from the next write onwards; nothing has
  to be registered by hand.
- **restore** — every ledger entry whose anchor still exists and whose comment is no longer in
  place is put back: whole-line runs inserted at the anchor, trailing comments re-appended to the
  line with its original column padding. Entries whose anchor line is gone, or whose text now
  appears in several sections and cannot be disambiguated, are skipped and logged — never guessed.

Three triggers, so no cooperation from the writer is required:

| Trigger | Path | When |
|---|---|---|
| systemd **path unit** | `hermes-config-guard.path` → `.service` | seconds after any write to a watched config |
| systemd **timer** | `hermes-config-guard.timer` (10 min) | backstop: a write that lands while the guard is mid-run |
| **wrapper** | `~/.hermes/bin/hermes-config-guarded` | on demand: capture → run the writer → restore, same second |

`hermes-config-guarded config set ...` is a drop-in for `hermes config set ...`; the wrapper is
for callers that want the comments back before the next path-unit tick (the mailbox cron worker).

## Why this is safe to leave unattended

- **It cannot change a value.** The only writes it performs are whole comment lines and trailing
  comments, and it refuses (exit 2, no write) unless the file with every comment removed is
  byte-identical before and after. Independently, it parses the result and refuses unless the
  *parsed value tree* is unchanged — and it never writes inside a block scalar (`notes: |`), where
  a `#` is content.
- **It is idempotent.** Comments already in place are left alone (a trailing comment is never
  doubled); a second run after a restore writes nothing — no mtime churn, no path-unit feedback
  loop.
- **It fails closed.** A ledger it cannot parse, an unreadable config, a stale anchor, or an
  ambiguous match aborts or skips that entry with a log line instead of guessing.
- **Everything is snapshotted and logged**: before modifying anything it copies the config to
  `~/.hermes/backups/config/config.yaml.comment-guard.<timestamp>` (5 kept), and every learn,
  restore, skip and refusal is appended to `~/.hermes/config-comments/guard.log`.
- **One writer at a time**: `flock` on `~/.hermes/config-comments/guard.lock` serialises the path
  unit, the timer and the wrapper.

## Files

| File | Role |
|---|---|
| `hermes-config-guard.py` | the guard: learn / restore / `--check` / `--run` / `--selftest` |
| `hermes-config-guarded` | wrapper for config-writing `hermes` commands |
| `hermes-config-guard.{service,path,timer}` | systemd user units (path + 10-minute backstop) |
| `targets.conf.example` | which configs to guard (copied to the state dir by `install.sh`) |
| `install.sh` | install to `~/.hermes/bin`, seed ledgers, enable the units, verify |

## Install / operate

```bash
ops/config-comment-guard/install.sh                     # install + enable + verify
python3 ~/.hermes/bin/hermes-config-guard.py --all --check      # exit 1 if any block is missing
python3 ~/.hermes/bin/hermes-config-guard.py --all --dry-run    # what a restore would insert
python3 ~/.hermes/bin/hermes-config-guard.py --selftest         # 12 built-in checks
journalctl --user -u hermes-config-guard.service -n 20          # what the units did
```

Adding a target (another profile) = one line in `~/.hermes/config-comments/targets.conf`, one
`PathChanged=` in the path unit, `systemctl --user daemon-reload && systemctl --user restart
hermes-config-guard.path`. The `tim` profile is deliberately **not** guarded: it is another
agent's profile.

## Limits

- An anchor key the writer *renames* strands its block (logged as a skip, kept in the ledger for
  the day the key comes back). Comments are never relocated speculatively.
- A trailing comment whose text now appears on several lines in *different* sections is only
  restored when the enclosing key path still matches; otherwise it is skipped and logged rather
  than attached to the wrong line.
- A comment block that is deliberately rewritten by a writer while the ledger still holds the old
  text will be restored *in addition* to the new text (both present). Editing the block means
  editing the ledger entry, or deleting it and letting learn pick the new one up.
- The guard is a Hermes **user-side** tool; it is not upstream. `hermes update` replaces
  `hermes-agent/`, not `~/.hermes/bin` or these units, so it survives updates.

## Evidence

`evidence/0041-before-after.txt` is the captured transcript of `evidence/verify-before-after.sh`:
the writer deleting 4 whole-line + 18 trailing comments, the guard putting them back, the systemd
service that fired the restore, the 14 self-test checks, and the MCP connectivity check.
