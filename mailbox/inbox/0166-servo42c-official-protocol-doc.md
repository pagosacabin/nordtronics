# 0166 — Pull the UART protocol doc from the official MKS-SERVO42C repo

## Context
Stephen shared the official repo: https://github.com/makerbase-mks/MKS-SERVO42C (default branch MKS-SERVO42C-V1.1). It holds `01_Makerbase SERVO42C Related documents/`, `User Manual/`, firmware, and an open-source `MKS SERVO42C CONTROL TOOL`. Juno could not list the documents directory from her side (API failures); you have web access — pull it yourself. This is the authoritative source that should settle the remaining open questions from the bench thread.

## Task
1. Find the serial/UART protocol documentation in the repo (likely under `01_Makerbase SERVO42C Related documents/` or `User Manual/`).
2. Answer definitively from that doc:
   a. What does command **0x33** actually return — working mode, or a move/pulse counter? (Your bench work treated it as a counter tracking ±800; a web table claimed working mode.)
   b. Is there a documented per-command pulse/distance/speed limit that explains the **2224-pulse ceiling** seen on 3200-pulse commands?
   c. Confirm the factory default UART baud (your research said 38400) and whether ANY command reads back the settings block (your 0165 probe said no).
3. Note anything else in the doc that contradicts your PROTOCOL.md — list each discrepancy.

## Success criteria
- a/b/c answered with doc quotes or section references, not guesses.
- Discrepancy list vs PROTOCOL.md, or "none found."

## Constraints
- Default model tier. Read-only: clone/fetch the public repo, change nothing.

## Proof
- Staged reply quoting the relevant doc sections (with file paths in the repo).

## Reply format
Staged reply per the mailbox README on main (protocol 1.0.0): summary, proof pointers, notes field.

expect-reply-within: 6h
