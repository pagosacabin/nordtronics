#!/usr/bin/env python3
"""Verification for mailbox task 0060 — worker cron moved to true DeepSeek off-peak.

Checks the live Hermes cron store (~/.hermes/profiles/cronrunner/cron/jobs.json):

1. exactly the four requested worker schedule lines exist, one job each;
2. no worker fire lands inside a DeepSeek peak window;
3. every off-peak :15 slot in a 14-day window is covered by exactly one worker job
   (no gap, no double-fire);
4. the 15-minute poller entry (job 0b32bc199a4e + the user-crontab tick line) is
   untouched.

Peak rule is the one the job's own pre-run script uses
(scripts/mailbox-protocol-check.py::peak_line): 01:00-04:00 and 06:00-10:00 UTC,
Monday-Friday. Local tz is America/Denver (MDT, UTC-6).
"""
from __future__ import annotations

import datetime as dt
import json
import pathlib
import subprocess
import sys

from croniter import croniter

JOBS = pathlib.Path("/home/astroboy/.hermes/profiles/cronrunner/cron/jobs.json")
WORKER_PREFIX = "Mailbox worker (DeepSeek)"
REQUESTED = {
    "15 4-18,22,23 * * 1-4": "Mon-Thu 4 AM-6 PM + 10-11 PM",
    "15 0-18,22,23 * * 0": "Sun midnight-6 PM + 10-11 PM",
    "15 4-23 * * 5": "Fri 4 AM-11 PM",
    "15 * * * 6": "Sat all day",
}
POLLER_EXPR = "*/15 * * * *"
TICKER_LINE = "cron tick --accept-hooks"


def is_peak(instant: dt.datetime) -> bool:
    """UTC Mon-Fri, 01:00-04:00 or 06:00-10:00 -> DeepSeek peak (double price)."""
    u = instant.astimezone(dt.timezone.utc)
    h = u.hour + u.minute / 60
    return u.weekday() < 5 and ((1 <= h < 4) or (6 <= h < 10))


def main() -> int:
    failures: list[str] = []
    store = json.loads(JOBS.read_text())
    jobs = store["jobs"]
    workers = [j for j in jobs if str(j.get("name", "")).startswith(WORKER_PREFIX)]
    pollers = [j for j in jobs if j["id"] == "0b32bc199a4e"]

    print("== worker jobs in the live store ==")
    for j in sorted(workers, key=lambda x: x["schedule"]["expr"]):
        print(f"  {j['id']}  {j['name']!r}  expr={j['schedule']['expr']!r}  "
              f"enabled={j['enabled']}  model={j['model']}  next={j['next_run_at']}")

    got = {j["schedule"]["expr"]: j for j in workers}
    for expr, label in REQUESTED.items():
        if expr not in got:
            failures.append(f"missing requested schedule {expr!r} ({label})")
        elif not got[expr]["enabled"]:
            failures.append(f"requested schedule {expr!r} exists but is disabled")
    for expr in got:
        if expr not in REQUESTED:
            failures.append(f"unexpected extra worker schedule {expr!r}")

    print("\n== next 3 fires per requested line (local time) ==")
    now = dt.datetime.now().replace(second=0, microsecond=0)
    for expr, label in REQUESTED.items():
        it = croniter(expr, now)
        nxt = [it.get_next(dt.datetime) for _ in range(3)]
        print(f"  {expr:<22} {label:<34} " + " | ".join(t.strftime("%a %Y-%m-%d %H:%M") for t in nxt))
        for t in nxt:
            if is_peak(t):
                failures.append(f"{expr!r} fires in a PEAK window at {t} ({t.astimezone(dt.timezone.utc)} UTC)")

    # 14-day sweep over every :15 slot (the job's only firing minute).
    start = dt.datetime(now.year, now.month, now.day) + dt.timedelta(days=1)
    horizon = start + dt.timedelta(days=14)
    slots = peak_slots = uncovered = double = 0
    t = start
    while t < horizon:
        inst = t.replace(minute=15)
        slots += 1
        covered = [e for e in REQUESTED if croniter.match(e, inst)]
        peak = is_peak(inst)
        peak_slots += 1 if peak else 0
        if peak and covered:
            failures.append(f"PEAK slot {inst} fires via {covered}")
        if not peak and len(covered) == 0:
            uncovered += 1
            if uncovered <= 5:
                failures.append(f"off-peak slot {inst} not covered by any worker line")
        if not peak and len(covered) > 1:
            double += 1
            if double <= 5:
                failures.append(f"off-peak slot {inst} covered twice: {covered}")
        t += dt.timedelta(hours=1)

    print(f"\n== 14-day sweep of :15 slots from {start:%Y-%m-%d} to {horizon:%Y-%m-%d} ==")
    print(f"  :15 slots swept          : {slots}")
    print(f"  of those, peak-window    : {peak_slots}  (worker fires there: 0)")
    print(f"  off-peak slots           : {slots - peak_slots}")
    print(f"  uncovered off-peak slots : {uncovered}")
    print(f"  double-covered off-peak  : {double}")

    print("\n== poller / ticker untouched ==")
    if len(pollers) != 1 or pollers[0]["schedule"]["expr"] != POLLER_EXPR:
        failures.append(f"poller job 0b32bc199a4e changed: {pollers}")
    else:
        print(f"  job 0b32bc199a4e 'Mailbox poll (Nvidia)' expr={POLLER_EXPR!r} "
              f"enabled={pollers[0]['enabled']} (unchanged)")
    crontab = subprocess.run(["crontab", "-l"], capture_output=True, text=True).stdout
    lines = [l for l in crontab.splitlines() if l.strip() and not l.lstrip().startswith("#")]
    tick = [l for l in lines if TICKER_LINE in l]
    if len(tick) != 1 or not tick[0].startswith("*/15 * * * *"):
        failures.append(f"crontab ticker line changed/missing: {lines}")
    else:
        print("  user crontab: exactly one */15 '* * * * *' tick line, unchanged")

    print("\n== verdict ==")
    if failures:
        for f in failures:
            print("  FAIL:", f)
        return 1
    print("  PASS — four requested lines live, zero peak fires, every off-peak :15 slot "
          "covered exactly once, poller/ticker untouched")
    return 0


if __name__ == "__main__":
    sys.exit(main())
