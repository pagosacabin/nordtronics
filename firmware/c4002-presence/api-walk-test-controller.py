"""0169 walk test controller — drives the bench test with spoken cues.

Read-only toward the board (ESPHome native API observer). Speaks cues on the
host's default audio sink so the person at the bench needs no screen.

Phases:
  A: cue "step out"  -> wait presence False (records hold from cue to clear)
  B: cue "walk in"   -> wait presence True, sample distance/speed/energy 20 s
  C: cue "step out"  -> wait presence False, confirm it stays False 60 s
  D: cue "done"

Writes a timeline log and a JSON summary next to this script's log.
"""
import asyncio
import datetime
import json
import statistics
import subprocess
import sys
import time

from aioesphomeapi import APIClient

HOST = "192.168.1.155"
CUE = "/home/astroboy/.hermes/cache/scratch/"
CUES = {
    "out": CUE + "cue1-out.mp3",
    "in": CUE + "cue3-in.mp3",
    "away": CUE + "cue4-away.mp3",
    "done": CUE + "cue5-done.mp3",
}
MAX_TOTAL = 900.0

T0 = time.time()
LOG: list[dict] = []
STATE: dict[str, object] = {}
TRANSITIONS: list[tuple[float, str, object, object]] = []
SAMPLES: list[dict] = []


def ts() -> str:
    return datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]


def log(kind: str, **kw) -> None:
    rec = {"t": round(time.time() - T0, 2), "clock": ts(), "kind": kind, **kw}
    LOG.append(rec)
    print(json.dumps(rec, default=str), flush=True)


def speak(name: str) -> None:
    path = CUES[name]
    log("cue", cue=name)
    try:
        subprocess.run(["paplay", path], check=True, timeout=30)
    except Exception as exc:  # noqa: BLE001 - audio is best-effort
        log("cue_failed", cue=name, error=str(exc))


def presence() -> object:
    return STATE.get("room_presence")


async def wait_for(value: bool, timeout: float) -> float | None:
    """Block until room_presence == value; return elapsed or None on timeout."""
    start = time.time()
    while time.time() - start < timeout and time.time() - T0 < MAX_TOTAL:
        if presence() is value:
            return time.time() - start
        await asyncio.sleep(0.2)
    return None


async def sample(seconds: float) -> None:
    end = time.time() + seconds
    while time.time() < end:
        SAMPLES.append({
            "t": round(time.time() - T0, 2),
            "presence": presence(),
            "out_pin": STATE.get("room_presence_out_pin"),
            "distance": STATE.get("target_distance"),
            "speed": STATE.get("target_speed"),
            "energy": STATE.get("target_energy"),
        })
        await asyncio.sleep(1.0)


async def main() -> int:
    cli = APIClient(HOST, 6053, None)
    await cli.connect(login=True)
    info = await cli.device_info()
    ents, _ = await cli.list_entities_services()
    names = {e.key: getattr(e, "object_id", str(e.key)) for e in ents}
    log("connect", host=HOST, name=info.name, mac=info.mac_address,
        esphome=info.esphome_version)

    def on_state(s):
        n = names.get(s.key, str(s.key))
        if n in ("room_presence", "room_presence_out_pin"):
            v = getattr(s, "state", None)
            prev = STATE.get(n)
            STATE[n] = v
            if prev != v:
                TRANSITIONS.append((round(time.time() - T0, 2), n, prev, v))
                log("transition", entity=n, was=prev, now=v)
        elif n in ("target_distance", "target_speed", "target_energy"):
            STATE[n] = getattr(s, "state", None)

    cli.subscribe_states(on_state)
    await asyncio.sleep(1.5)

    result: dict[str, object] = {}

    # A: clear the sensor
    log("phase", phase="A", note="person should be in view now",
        presence=presence())
    speak("out")
    t_a = time.time()
    elapsed = await wait_for(False, 240)
    result["A_clear"] = elapsed
    log("phase_result", phase="A", cleared=elapsed is not None,
        seconds_from_cue=round(time.time() - t_a, 2) if elapsed is not None else None)

    # B: walk in
    speak("in")
    t_b = time.time()
    elapsed = await wait_for(True, 180)
    result["B_present"] = round(time.time() - t_b, 2) if elapsed is not None else None
    log("phase_result", phase="B", present=elapsed is not None,
        seconds_from_cue=result["B_present"])
    await sample(20.0)

    # C: leave and stay out
    speak("away")
    t_c = time.time()
    elapsed = await wait_for(False, 240)
    result["C_clear"] = round(time.time() - t_c, 2) if elapsed is not None else None
    log("phase_result", phase="C", cleared=elapsed is not None,
        seconds_from_cue=result["C_clear"])
    stable = await wait_for(False, 60) is not None
    result["C_stable_60s"] = stable
    log("phase_result", phase="C", stayed_clear_60s=stable)

    speak("done")

    # summary
    dists = [s["distance"] for s in SAMPLES if isinstance(s["distance"], float)]
    speeds = [s["speed"] for s in SAMPLES if isinstance(s["speed"], float)]
    energies = [s["energy"] for s in SAMPLES if isinstance(s["energy"], float)]
    agree = all(s["presence"] == s["out_pin"] for s in SAMPLES)
    result.update({
        "samples": len(SAMPLES),
        "distance_m": {"min": min(dists) if dists else None,
                       "max": max(dists) if dists else None,
                       "mean": round(statistics.fmean(dists), 3) if dists else None},
        "speed_ms": {"min": min(speeds) if speeds else None,
                     "max": max(speeds) if speeds else None},
        "energy": {"min": min(energies) if energies else None,
                   "max": max(energies) if energies else None},
        "out_pin_agrees_while_present": agree,
        "transitions": TRANSITIONS,
    })
    log("summary", **result)
    with open("/home/astroboy/.hermes/cache/scratch/0169-walktest-result.json", "w") as fh:
        json.dump({"result": result, "log": LOG, "samples": SAMPLES}, fh, indent=2,
                  default=str)
    await cli.disconnect()
    return 0


sys.exit(asyncio.run(main()))
