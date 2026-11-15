# 0191 — Writing app rung 2: DeepSeek key to the VPS + live smoke

## Assignment
- **Hermes:** execute and stage the reply. The key transfer is yours alone — the key lives on the laptop, and Juno has no path to it.
- **Juno:** already deployed the app on the VPS (details below); verifies your staged reply, closes the task.
- **Stephen:** adding the Cloudflare DNS record himself (parallel, not your concern).

## Context
Rung 2 of Bethany's writing app (0186, merged to cabin-play main @ 175ac49). Juno has deployed it on the VPS:

- Code: `/home/deploy/cabin-play/writing-peer-review` (public clone, venv installed)
- Service: systemd `writing-review`, uvicorn on `127.0.0.1:8765`, active and answering 200
- nginx vhost `writing.nordtronics.io` with basic auth in front (Juno holds the credential; you do not need it — your smoke runs on localhost, inside the gate)
- The app currently has **no `.env` on the VPS**, so reviews fall back to the STUB provider. That is the one gap this task closes.

Stephen's authorization for using Hermes's DeepSeek key in this app stands (2026-11-15, main chat) — the test worked, and he said go on rung 2.

## Task
1. **Transfer the key, laptop → VPS, value never exposed.** From your laptop checkout of `writing-peer-review/` (the 0186 build tree, whose `.env` holds the working key), extract only the `DEEPSEEK_API_KEY=...` line and write it to the VPS at `/home/deploy/cabin-play/writing-peer-review/.env` over your existing SSH access as `deploy`. Then set it `chmod 600`, owned by `deploy`.
2. **Restart the service:** `sudo systemctl restart writing-review` on the VPS; confirm `systemctl is-active writing-review` = active.
3. **Live smoke on the VPS, via localhost** (`http://127.0.0.1:8765`, bypassing nginx): run the same flow your `tests/live_smoke.py` exercises — create a piece, submit it for review — and confirm the returned review's provider is **`deepseek`** (live, not stub), with the four sections present. Also confirm `GET /` on localhost returns the app with the peak/off-peak strip.

## Constraints
- **The key value never appears** in this task's reply, in any log you quote, in shell history you paste, or anywhere in the repo. Verify presence by name and count only (e.g. `grep -c '^DEEPSEEK_API_KEY='`), never by printing the line.
- No code changes, no repo writes, no DNS or certificate work — Juno and Stephen hold those.
- If the laptop `.env` is missing or the key in it no longer works (API rejects it), stop and report exactly that — do not substitute any other key.

## Success criteria
1. VPS `.env` exists at the path above, mode 600, contains exactly one `DEEPSEEK_API_KEY=` line (count reported, value never shown).
2. `writing-review` restarted and active after the transfer.
3. Localhost smoke returns a review with provider `deepseek` and all four sections — quoted in the reply with the piece text you used (use a throwaway test piece, not anyone's real writing).

## Cost
Worker time only (DeepSeek flash). No Newton session.
