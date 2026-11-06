"""0169 walk test capture — read-only ESPHome API observer.

Logs every presence transition with a wall-clock timestamp, plus a 3 s heartbeat
of distance/speed/energy while a target is present. Reconnects if the board
drops. Makes no changes to the device.
"""
import asyncio
import datetime
import sys
import time

from aioesphomeapi import APIClient

HOST = "192.168.1.155"
DURATION = float(sys.argv[1]) if len(sys.argv) > 1 else 900.0
INTEREST = ("room_presence", "room_presence_out_pin",
            "target_distance", "target_speed", "target_energy")


def ts() -> str:
    return datetime.datetime.now().strftime("%H:%M:%S.%f")[:-3]


async def run_once(deadline: float) -> None:
    cli = APIClient(HOST, 6053, None)
    t0 = time.time()
    await cli.connect(login=True)
    info = await cli.device_info()
    print(f"{ts()} CONNECT {HOST}:6053 {info.name} mac={info.mac_address} "
          f"esphome={info.esphome_version} in {time.time()-t0:.2f}s", flush=True)

    ents, _ = await cli.list_entities_services()
    names = {e.key: getattr(e, "object_id", str(e.key)) for e in ents}
    state: dict[str, object] = {}
    last_hb = 0.0

    def on_state(s):
        nonlocal last_hb
        n = names.get(s.key, str(s.key))
        if n not in INTEREST:
            return
        v = getattr(s, "state", None)
        prev = state.get(n, "<none>")
        state[n] = v
        if n in ("room_presence", "room_presence_out_pin") and prev != v:
            print(f"{ts()} TRANSITION {n}: {prev} -> {v}", flush=True)
        now = time.time()
        if now - last_hb >= 3.0:
            last_hb = now
            print(f"{ts()} hb presence={state.get('room_presence')!s:5} "
                  f"out_pin={state.get('room_presence_out_pin')!s:5} "
                  f"dist={state.get('target_distance')} "
                  f"speed={state.get('target_speed')} "
                  f"energy={state.get('target_energy')}", flush=True)

    cli.subscribe_states(on_state)
    while time.time() < deadline:
        await asyncio.sleep(1.0)
    await cli.disconnect()
    print(f"{ts()} DISCONNECT (window end)", flush=True)


async def main() -> int:
    deadline = time.time() + DURATION
    print(f"{ts()} START window={DURATION:.0f}s", flush=True)
    while time.time() < deadline:
        try:
            await run_once(deadline)
        except Exception as exc:  # noqa: BLE001 - keep observing
            print(f"{ts()} RECONNECT after {type(exc).__name__}: {exc}", flush=True)
            await asyncio.sleep(3)
    print(f"{ts()} END", flush=True)
    return 0


sys.exit(asyncio.run(main()))
