#!/usr/bin/env python3
"""Restore explanatory comment blocks that `hermes config set` (and friends) strip.

`hermes config set` reads the user config, mutates the tree and re-dumps it with
`yaml.safe_dump` (hermes_cli/config.py::_write_user_config -> atomic_yaml_write). Comments
are not part of the parsed tree, so **every** config write drops every comment line — the
FreeCAD ABI note, the Flatpak TMPDIR note, and the commented-out documentation blocks.

This guard keeps a *ledger* of comment blocks outside the config, keyed by the config key
each block annotates, and re-injects any block that has gone missing. It is deliberately
incapable of changing a value: the only operation it performs is inserting whole comment
lines, and it refuses to write unless the file with all comment lines removed is
byte-identical before and after (see `assert_values_unchanged`).

Modes
    (default)          learn new comment blocks, then restore missing ones
    --capture          learn only (seed/extend the ledger from the live file)
    --check            exit 1 if any ledger block is missing; never writes
    --run -- CMD ...   run CMD (a config writer), then restore
    --selftest         run the built-in tests and exit

Targets
    -c/--config PATH   operate on one config (ledger beside it in the state dir), or
    --all              every target in <state>/targets.conf

State (defaults, override with --state)
    <state>/targets.conf        config_path<TAB>ledger_path  (one per line, # comments ok)
    <state>/ledger-<slug>.yaml  the comment ledger per config
    <state>/guard.log           every action, with timestamps
    <state>/guard.lock          flock, so concurrent triggers serialise

Exit codes: 0 ok/no-op, 1 --check found missing blocks, 2 refused (unsafe to write).
"""

from __future__ import annotations

import argparse
import fcntl
import hashlib
import os
import re
import shlex
import shutil
import subprocess
import sys
import time
from pathlib import Path

try:  # optional: only used to sanity-check that the file still parses
    import yaml  # type: ignore
except Exception:  # pragma: no cover - degrade to the textual invariant
    yaml = None

DEFAULT_STATE = Path.home() / ".hermes" / "config-comments"
COMMENT_RE = re.compile(r"^[ \t]*#")
# "key:" / "key: value" at any indent; not a comment, not a bare list dash. The key part may
# not contain ':' or '#' — otherwise `- delegation   # optional: split ...` parses as a key
# named "delegation   # optional" and pollutes the key path used for anchor lookup.
KEY_RE = re.compile(r"^([ \t]*)([^ \t#:\-][^#]*?)[ \t]*:([ \t].*|$)")
LIST_ITEM_KEY_RE = re.compile(r"^([ \t]*)-[ \t]+([^ \t#:\-][^#]*?)[ \t]*:([ \t].*|$)")
BLOCK_SCALAR_RE = re.compile(r"^[|>][+-]?[0-9]*$")
MAX_BACKUPS = 5


# --------------------------------------------------------------------------- helpers


def log(state: Path, msg: str) -> None:
    stamp = time.strftime("%Y-%m-%dT%H:%M:%S%z")
    line = f"{stamp} {msg}\n"
    try:
        state.mkdir(parents=True, exist_ok=True)
        with (state / "guard.log").open("a", encoding="utf-8") as fh:
            fh.write(line)
    except OSError:
        pass
    print(line.rstrip())


def read_lines(path: Path) -> list[str]:
    return path.read_text(encoding="utf-8").split("\n")


def indent_of(line: str) -> int:
    return len(line) - len(line.lstrip(" "))


def slug_for(path: Path) -> str:
    s = str(path).replace(str(Path.home()), "home").lstrip("/")
    for suffix in (".yaml", ".yml"):
        if s.endswith(suffix):
            s = s[: -len(suffix)]
            break
    return re.sub(r"[^A-Za-z0-9.]+", "-", s).strip("-")


def is_comment(line: str) -> bool:
    return bool(COMMENT_RE.match(line))


def strip_comment_lines(lines: list[str]) -> list[str]:
    return [ln for ln in lines if not is_comment(ln)]


def split_trailing_comment(line: str) -> tuple[str, str]:
    """('value part', '# comment') for a line with a trailing comment; ('line', '') otherwise.

    Quote-aware: a '#' inside a quoted scalar is content, and a '#' not preceded by
    whitespace (e.g. `a#b`) is not a YAML comment.
    """
    single = double = False
    i = 0
    while i < len(line):
        ch = line[i]
        if double:
            if ch == "\\":
                i += 2
                continue
            if ch == '"':
                double = False
        elif single:
            if ch == "'":
                if i + 1 < len(line) and line[i + 1] == "'":
                    i += 2
                    continue
                single = False
        elif ch == '"':
            double = True
        elif ch == "'":
            single = True
        elif ch == "#" and (i == 0 or line[i - 1] in " \t"):
            return line[:i].rstrip(), line[i:].rstrip()
        i += 1
    return line.rstrip(), ""


def non_comment_signature(lines: list[str]) -> list[str]:
    """The file with every whole-line *and* trailing comment removed — the invariant check."""
    opaque = opaque_regions(lines)
    out = []
    for idx, line in enumerate(lines):
        if not line.strip() or is_comment(line):
            continue
        value = line if idx in opaque else split_trailing_comment(line)[0]
        out.append(value.rstrip())
    return out


def opaque_regions(lines: list[str]) -> set[int]:
    """Line indexes inside a block scalar body (`notes: |`), which must stay untouched."""
    opaque: set[int] = set()
    i = 0
    while i < len(lines):
        line = lines[i]
        m = KEY_RE.match(line) or LIST_ITEM_KEY_RE.match(line)
        if m and not is_comment(line):
            if BLOCK_SCALAR_RE.match(m.group(3).strip()):
                base = indent_of(line)
                j = i + 1
                while j < len(lines):
                    if lines[j].strip() and indent_of(lines[j]) <= base:
                        break
                    opaque.add(j)
                    j += 1
                i = j
                continue
        i += 1
    return opaque


def scan_structure(lines: list[str]) -> tuple[list[tuple[int, tuple[str, ...], int, str]], dict[int, tuple[str, ...]]]:
    """(key lines, enclosing key path per line).

    `lines` (the line's own key path when it declares a key, else the path of the nearest
    enclosing mapping key) — needed because `- delegation` appears in more than one list and a
    comment must go back to the right one.
    """
    opaque = opaque_regions(lines)
    stack: list[tuple[int, str]] = []
    keys: list[tuple[int, tuple[str, ...], int, str]] = []
    paths: dict[int, tuple[str, ...]] = {}
    for idx, line in enumerate(lines):
        if idx in opaque or not line.strip() or is_comment(line):
            continue
        m = KEY_RE.match(line)
        indent, key = indent_of(line), None
        if m:
            key = m.group(2).strip()
        else:
            m2 = LIST_ITEM_KEY_RE.match(line)
            if m2:  # `- key: value` — treat as a key two columns deeper than the dash
                indent, key = indent_of(line) + 2, m2.group(2).strip()
        if key is not None:
            while stack and indent <= stack[-1][0]:
                stack.pop()
            stack.append((indent, key))
            path = tuple(k for _, k in stack)
            keys.append((idx, path, indent, key))
            paths[idx] = path
        else:
            paths[idx] = tuple(k for _, k in stack)
    return keys, paths


def scan_keys(lines: list[str]) -> list[tuple[int, tuple[str, ...], int, str]]:
    return scan_structure(lines)[0]


def find_anchor(
    lines: list[str], keys: list[tuple[int, tuple[str, ...], int, str]], anchor: str, position: str
) -> int | None:
    """Line index to insert at, or None when the anchor key is absent."""
    if position == "eof":
        i = len(lines)
        while i > 0 and not lines[i - 1].strip():
            i -= 1
        return i
    path = tuple(anchor.split("."))
    hit = next((k for k in keys if k[1] == path), None)
    if hit is None:
        return None
    idx, _, indent, _ = hit
    if position == "before":
        return idx
    last = idx
    j = idx + 1
    while j < len(lines):
        line = lines[j]
        if line.strip() and indent_of(line) <= indent:
            break
        if line.strip():
            last = j
        j += 1
    return last + 1


def block_id(block: dict) -> str:
    if block.get("kind") == "inline_comment":
        raw = f"inline|{block.get('path')}|{block.get('line')}|{block.get('trailing')}"
    else:
        raw = f"{block.get('anchor')}|{block.get('position')}|" + "\n".join(block["lines"])
    return hashlib.sha1(raw.encode("utf-8")).hexdigest()[:12]


def normalise_block(anchor, position, lines, indent=0):
    """Comment lines re-indented to `indent`, so a block survives re-nesting."""
    out = []
    for ln in lines:
        body = ln.strip()
        out.append((" " * indent + body) if body else ln)
    return out


def already_present(lines: list[str], at: int, block_lines: list[str]) -> bool:
    """All of the block's comment lines already sit within 15 lines above the insertion point."""
    near = {
        ln.strip()
        for ln in lines[max(0, at - 15):at]
        if is_comment(ln)
    }
    return all(ln.strip() in near for ln in block_lines)


def values_unchanged(before: list[str], after: list[str]) -> bool:
    """True when the two files differ only by comment text (whole-line and trailing)."""
    return non_comment_signature(before) == non_comment_signature(after)


# --------------------------------------------------------------------------- ledger


def ledger_path(state: Path, config: Path, explicit: str | None) -> Path:
    if explicit:
        return Path(explicit)
    return state / f"ledger-{slug_for(config)}.yaml"


def load_ledger(path: Path) -> dict:
    if not path.exists():
        return {"version": 1, "config": None, "blocks": []}
    text = path.read_text(encoding="utf-8")
    try:
        if yaml is not None:
            data = yaml.safe_load(text)
        else:
            import json

            data = json.loads(text)
    except Exception as exc:
        raise SystemExit(f"refusing: ledger {path} is unreadable: {exc}")
    if not isinstance(data, dict) or not isinstance(data.get("blocks", []), list):
        raise SystemExit(f"refusing: ledger {path} has no 'blocks' list")
    data.setdefault("version", 1)
    data.setdefault("config", str(path))
    return data


def save_ledger(path: Path, data: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if yaml is not None:
        body = yaml.safe_dump(data, sort_keys=False, allow_unicode=True, width=100)
    else:
        import json

        body = json.dumps(data, indent=2, ensure_ascii=False) + "\n"
    tmp = path.with_suffix(path.suffix + ".tmp")
    tmp.write_text(body, encoding="utf-8")
    os.chmod(tmp, 0o600)
    os.replace(tmp, path)


def capture_blocks(lines: list[str]) -> list[dict]:
    """Every maximal run of comment lines plus every trailing comment, with its anchor."""
    opaque = opaque_regions(lines)
    keys, line_paths = scan_structure(lines)
    blocks: list[dict] = []
    by_index = {k[0]: k for k in keys}
    i = 0
    while i < len(lines):
        if i in opaque or not is_comment(lines[i]):
            i += 1
            continue
        start = i
        while i < len(lines) and is_comment(lines[i]):
            i += 1
        end = i
        run_indent = indent_of(lines[start])
        nxt = end
        while nxt < len(lines) and not lines[nxt].strip():
            nxt += 1
        anchor = position = None
        nk = by_index.get(nxt)
        if nk is not None and nk[2] >= run_indent:
            anchor, position = ".".join(nk[1]), "before"
        else:
            prev = [k for k in keys if k[0] < start]
            if prev:
                anchor, position = ".".join(prev[-1][1]), "after"
            else:
                anchor, position = None, "eof"
        blocks.append(
            {
                "kind": "comment_block",
                "anchor": anchor,
                "position": position,
                "indent": (by_index[nxt][2] if position == "before" else run_indent),
                "lines": lines[start:end],
                "captured": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
            }
        )
    for idx, line in enumerate(lines):
        if idx in opaque or not line.strip() or is_comment(line):
            continue
        value, trailing = split_trailing_comment(line)
        if not trailing:
            continue
        path = by_index.get(idx)
        blocks.append(
            {
                "kind": "inline_comment",
                "anchor": ".".join(path[1]) if path else None,
                "path": ".".join(line_paths.get(idx, ())),
                "line": value.strip(),
                "pad": max(2, len(line[: line.rindex(trailing)]) - len(value.rstrip())),
                "trailing": trailing,
                "captured": time.strftime("%Y-%m-%dT%H:%M:%S%z"),
            }
        )
    return blocks


def merge_ledger(ledger: dict, found: list[dict], config: Path, state: Path) -> int:
    have = {block_id(b): b for b in ledger["blocks"]}
    added = 0
    for block in found:
        bid = block_id(block)
        if bid in have:
            continue
        if block.get("kind") == "inline_comment" and block.get("path"):
            # Upgrade a pathless twin (learned by an older guard) instead of duplicating it:
            # two entries for the same line would both try to restore the same comment.
            twin = next(
                (
                    b
                    for b in ledger["blocks"]
                    if b.get("kind") == "inline_comment"
                    and not b.get("path")
                    and b.get("line") == block.get("line")
                    and b.get("trailing") == block.get("trailing")
                ),
                None,
            )
            if twin is not None:
                ledger["blocks"].remove(twin)
                have.pop(block_id(twin), None)
        block["id"] = bid
        ledger["blocks"].append(block)
        have[bid] = block
        added += 1
        if block.get("kind") == "inline_comment":
            log(state, f"learned  [{config.name}] inline {block.get('anchor')}: "
                       f"{block['line'][:50]!r} + {block['trailing'][:40]!r}")
        else:
            log(state, f"learned  [{config.name}] {block['position']:6} {block['anchor']}: "
                       f"{block['lines'][0].strip()[:60]!r}")
    if added:
        ledger["config"] = str(config)
        ledger["version"] = 1
    return added


# --------------------------------------------------------------------------- restore


def plan_restore(lines: list[str], ledger: dict) -> tuple[list, list, list]:
    """Plan (insertions, trailing-comment appends, skipped-block notes)."""
    keys, line_paths = scan_structure(lines)
    ops: list[tuple[int, list[str]]] = []
    inline_ops: list[tuple[int, str]] = []
    missing: list[str] = []
    for block in ledger["blocks"]:
        if block.get("kind") == "inline_comment":
            target = block.get("line", "")
            want_path = block.get("path", "")
            candidates = []
            for idx, line in enumerate(lines):
                if not line.strip() or is_comment(line):
                    continue
                value, trailing = split_trailing_comment(line)
                if value.strip() == target:
                    candidates.append((idx, trailing))
            hit = next((c for c in candidates if ".".join(line_paths.get(c[0], ())) == want_path), None)
            if hit is None and len(candidates) == 1 and not want_path:
                hit = candidates[0]  # ledger entry predates path anchoring: only safe if unique
            if hit is None:
                why = ("same text appears in several sections"
                       if len(candidates) > 1 else f"line {target!r} absent")
                missing.append(f"{block.get('anchor')} ({why})")
                continue
            idx, trailing = hit
            if trailing:  # a comment is already there; never stack a second one
                continue
            # reuse the original column padding so re-adding a comment reproduces the old bytes
            pad = " " * max(2, int(block.get("pad", 2)))
            inline_ops.append((idx, f"{lines[idx].rstrip()}{pad}{block['trailing']}"))
            continue
        anchor, position = block.get("anchor"), block.get("position", "before")
        at = find_anchor(lines, keys, anchor, position)
        if at is None:
            missing.append(f"{anchor} (anchor key absent)")
            continue
        explicit_indent = block.get("indent")
        if position == "eof":
            indent = 0
        elif position == "before":
            indent = indent_of(lines[at])
        else:  # after: match the last body line of the anchor block
            probe = at - 1
            while probe >= 0 and not lines[probe].strip():
                probe -= 1
            indent = indent_of(lines[probe]) if probe >= 0 else 0
        if explicit_indent is not None and position == "before" and not lines[at].strip():
            indent = explicit_indent
        want = normalise_block(anchor, position, block["lines"], indent)
        if already_present(lines, at, want):
            continue
        ops.append((at, want))
    ops.sort(key=lambda op: op[0])
    return ops, inline_ops, missing


def backup_config(config: Path) -> Path | None:
    root = config.parent / "backups" / "config"
    try:
        root.mkdir(parents=True, exist_ok=True)
        dest = root / f"{config.name}.comment-guard.{time.strftime('%Y%m%d-%H%M%S')}"
        if dest.exists():
            return None
        shutil.copy2(config, dest)
        existing = sorted(
            (p for p in root.iterdir() if p.name.startswith(f"{config.name}.comment-guard.")),
            key=lambda p: p.name,
            reverse=True,
        )
        for stale in existing[MAX_BACKUPS:]:
            stale.unlink(missing_ok=True)
        return dest
    except OSError:
        return None


def process(
    config: Path,
    ledger_file: Path,
    state: Path,
    *,
    learn: bool = True,
    dry_run: bool = False,
    check_only: bool = False,
) -> int:
    if not config.is_file():
        log(state, f"skip     [{config}] not a file")
        return 0
    ledger = load_ledger(ledger_file)
    lines = read_lines(config)
    if learn and not check_only and not dry_run:
        if merge_ledger(ledger, capture_blocks(lines), config, state):
            save_ledger(ledger_file, ledger)
    ops, inline_ops, missing = plan_restore(lines, ledger)
    for name in missing:
        log(state, f"warn     [{config.name}] ledger block skipped: {name}")
    if check_only:
        if ops or inline_ops:
            log(state, f"MISSING  [{config}] {len(ops)} comment block(s) and "
                       f"{len(inline_ops)} trailing comment(s) absent")
            return 1
        log(state, f"ok       [{config}] all {len(ledger['blocks'])} ledger block(s) present")
        return 0
    if not ops and not inline_ops:
        log(state, f"ok       [{config}] {len(ledger['blocks'])} block(s) in ledger, nothing to restore")
        return 0
    if dry_run:
        log(state, f"dry-run  [{config}] would insert {len(ops)} block(s) / "
                   f"{sum(len(o[1]) for o in ops)} line(s) and append {len(inline_ops)} "
                   f"trailing comment(s)")
        return 0

    out = list(lines)
    for idx, text in inline_ops:  # trailing comments first: no line-count change
        out[idx] = text
    for at, block_lines in sorted(ops, key=lambda op: op[0], reverse=True):
        out[at:at] = block_lines
    if not values_unchanged(lines, out):
        log(state, f"REFUSED  [{config}] restore would change non-comment content — no write")
        return 2
    if yaml is not None:
        try:
            if yaml.safe_load("\n".join(out)) != yaml.safe_load("\n".join(lines)):
                log(state, f"REFUSED  [{config}] restore changed the parsed value tree — no write")
                return 2
        except Exception as exc:
            log(state, f"REFUSED  [{config}] restore produced invalid YAML: {exc} — no write")
            return 2
    backup = backup_config(config)
    mode = config.stat().st_mode & 0o7777
    tmp = config.with_suffix(config.suffix + ".guard-tmp")
    tmp.write_text("\n".join(out), encoding="utf-8")
    os.chmod(tmp, mode)
    os.replace(tmp, config)
    log(state, f"restored [{config}] {len(ops)} block(s) / {sum(len(o[1]) for o in ops)} comment "
               f"line(s) + {len(inline_ops)} trailing comment(s); backup={backup if backup else 'n/a'}")
    return 0


def load_targets(state: Path) -> list[tuple[Path, Path]]:
    conf = state / "targets.conf"
    if not conf.exists():
        return []
    out = []
    for raw in conf.read_text(encoding="utf-8").splitlines():
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        parts = line.split()
        config = Path(os.path.expanduser(parts[0]))
        ledger = Path(os.path.expanduser(parts[1])) if len(parts) > 1 else ledger_path(state, config, None)
        out.append((config, ledger))
    return out


def run_and_restore(config: Path, ledger_file: Path, state: Path, cmd: list[str]) -> int:
    """Capture, run a config writer, then restore. Used by the `hermes-config-guarded` wrapper."""
    lines = read_lines(config) if config.is_file() else []
    if lines:
        ledger = load_ledger(ledger_file)
        if merge_ledger(ledger, capture_blocks(lines), config, state):
            save_ledger(ledger_file, ledger)
    log(state, f"run      [{config}] {' '.join(shlex.quote(c) for c in cmd)}")
    proc = subprocess.run(cmd)
    if proc.returncode != 0:
        log(state, f"writer exit {proc.returncode}; restoring anyway")
    after = process(config, ledger_file, state, learn=True)
    return after or proc.returncode


# --------------------------------------------------------------------------- targets


def cmd_targets(state: Path, args) -> int:
    if args.selftest:
        return selftest()
    if args.all:
        targets = load_targets(state)
        if not targets:
            log(state, f"no targets in {state / 'targets.conf'}")
            return 0
    elif args.config:
        config = Path(args.config).expanduser()
        targets = [(config, ledger_path(state, config, args.ledger))]
    else:
        print(__doc__)
        return 0
    rc = 0
    for config, ledger in targets:
        if args.run:
            rc = max(rc, run_and_restore(config, ledger, state, args.run))
        else:
            rc = max(rc, process(config, ledger, state,
                                 learn=not args.no_learn, dry_run=args.dry_run,
                                 check_only=args.check))
    return rc


# --------------------------------------------------------------------------- selftest


SAMPLE_LIVE = """\
model:
  default: deepseek-ai/deepseek-v4.1-flash
mcp_servers:
  kicad:
    command: node
    env:
      KICAD_PATH: /var/lib/flatpak/exports/bin/org.kicad.KiCad
      # Flatpak KiCad has a private /tmp; scratch copies must live under $HOME
      TMPDIR: /home/astroboy/.cache/kicad-mcp/tmp
      KICAD_MCP_LOG_LEVEL: INFO        # keep the server chatter out of the tick log
  freecad:
    # FreeCAD Robust MCP server, embedded headless mode. Launcher runs the
    # server under the FreeCAD snap's own Python 3.12 (ABI-bound).
    command: /home/astroboy/.local/bin/freecad-mcp-robust
    env:
      FREECAD_MODE: embedded
notes: |
  # not a comment: inside a block scalar
  keep me
"""

SAMPLE_STRIPPED = """\
model:
  default: deepseek-ai/deepseek-v4.1-flash
mcp_servers:
  kicad:
    command: node
    env:
      KICAD_PATH: /var/lib/flatpak/exports/bin/org.kicad.KiCad
      TMPDIR: /home/astroboy/.cache/kicad-mcp/tmp
      KICAD_MCP_LOG_LEVEL: INFO
  freecad:
    command: /home/astroboy/.local/bin/freecad-mcp-robust
    env:
      FREECAD_MODE: embedded
notes: |
  # not a comment: inside a block scalar
  keep me
"""


def selftest() -> int:
    import tempfile

    failures = []
    with tempfile.TemporaryDirectory() as tmp:
        state = Path(tmp) / "state"
        state.mkdir()
        config = Path(tmp) / "config.yaml"
        ledger = state / "ledger-test.yaml"
        checks = []

        config.write_text(SAMPLE_LIVE, encoding="utf-8")
        rc = process(config, ledger, state, learn=True)
        checks.append(("learn on a comment-bearing config is a no-op write", rc == 0))
        kinds = [b.get("kind") for b in load_ledger(ledger)["blocks"]]
        checks.append(("ledger seeded with 2 comment blocks + 1 trailing comment",
                       kinds.count("comment_block") == 2 and kinds.count("inline_comment") == 1))

        # simulate the writer: drop every comment line, as yaml.safe_dump does
        config.write_text(SAMPLE_STRIPPED, encoding="utf-8")
        written = non_comment_signature(read_lines(config))
        rc = process(config, ledger, state, learn=True)
        restored = config.read_text(encoding="utf-8")
        checks.append(("restore rewrites the stripped config", rc == 0))
        checks.append(("FreeCAD block back and re-indented",
                       "    # FreeCAD Robust MCP server" in restored
                       and "    # server under the FreeCAD snap's own Python 3.12" in restored))
        checks.append(("TMPDIR block back at its key",
                       "      # Flatpak KiCad has a private /tmp"
                       in restored.replace(";", ";")))
        checks.append(("block scalar body untouched",
                       "  # not a comment: inside a block scalar\n  keep me" in restored))
        checks.append(("trailing comment restored, once, with its original padding",
                       restored.count("# keep the server chatter out of the tick log") == 1
                       and "KICAD_MCP_LOG_LEVEL: INFO        # keep" in restored))
        checks.append(("idempotent second restore",
                       process(config, ledger, state, learn=True) == 0
                       and config.read_text(encoding="utf-8") == restored))

        # values-preservation invariant: the restore added comments and nothing else
        checks.append(("no non-comment content changed by the restore",
                       non_comment_signature(read_lines(config)) == written))

        # a writer that reorders keys must not break anchoring
        reordered = ("mcp_servers:\n  freecad:\n    env:\n      FREECAD_MODE: embedded\n"
                     "    command: /x\n  kicad:\n    env:\n      TMPDIR: /t\n")
        config.write_text(reordered, encoding="utf-8")
        rc = process(config, ledger, state, learn=True)
        got = config.read_text(encoding="utf-8")
        checks.append(("anchors resolve after a reordering writer",
                       rc == 0 and "# FreeCAD Robust" in got and "# Flatpak KiCad" in got
                       and "      TMPDIR: /t" in got))

        # --check reports a stripped file and writes nothing
        config.write_text(SAMPLE_STRIPPED, encoding="utf-8")
        rc = process(config, ledger, state, learn=False, check_only=True)
        checks.append(("--check exits 1 on a stripped config",
                       rc == 1 and config.read_text(encoding="utf-8") == SAMPLE_STRIPPED))

        # an item listed in two sections: its comment must go back to the right section
        dup = ("platform_toolsets:\n  cli:\n    - a2a\n    - browser\n  cron:\n"
               "    - a2a  # interactive-only surface\n    - browser\n")
        config.write_text(dup, encoding="utf-8")
        process(config, ledger, state, learn=True)
        config.write_text(dup.replace("  # interactive-only surface", ""), encoding="utf-8")
        process(config, ledger, state, learn=True)
        checks.append(("ambiguous item — comment restored to the right section",
                       config.read_text(encoding="utf-8") == dup))

        # corrupt YAML is refused, not clobbered
        config.write_text(SAMPLE_STRIPPED.replace("model:", "model: [unclosed\n"), encoding="utf-8")
        before_corrupt = config.read_text(encoding="utf-8")
        rc = process(config, ledger, state, learn=False)
        checks.append(("invalid YAML refused with exit 2",
                       rc == 2 and config.read_text(encoding="utf-8") == before_corrupt))

        # ledger block whose anchor is gone is skipped, not misplaced
        config.write_text("unrelated: 1\n", encoding="utf-8")
        process(config, ledger, state, learn=False)
        checks.append(("absent anchors are skipped",
                       config.read_text(encoding="utf-8") == "unrelated: 1\n"))

        for name, ok in checks:
            print(("PASS  " if ok else "FAIL  ") + name)
            if not ok:
                failures.append(name)
    print(f"\n{len(failures)} failing of {len(checks)} checks")
    return 1 if failures else 0


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(add_help=True)
    ap.add_argument("-c", "--config")
    ap.add_argument("--ledger")
    ap.add_argument("--state", default=str(DEFAULT_STATE))
    ap.add_argument("--all", action="store_true", help="every target in <state>/targets.conf")
    ap.add_argument("--capture", action="store_true", help="learn new blocks; never restores")
    ap.add_argument("--no-learn", action="store_true")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--selftest", action="store_true")
    ap.add_argument("--run", nargs=argparse.REMAINDER, metavar="CMD ...")
    args = ap.parse_args(argv)

    if args.selftest:
        return selftest()
    state = Path(args.state).expanduser()
    state.mkdir(parents=True, exist_ok=True)
    if args.capture:
        args.dry_run = False
        args.check = False
    if args.run and args.run[0] == "--":
        args.run = args.run[1:]

    with (state / "guard.lock").open("w") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        if args.run and not args.run:
            return 2
        return cmd_targets(state, args)


if __name__ == "__main__":
    sys.exit(main())
