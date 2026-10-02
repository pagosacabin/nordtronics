"""Validation and mapping for Watch/alert events on `<root>/<base-id>/events`.

Task 0095 splits the job in two:

* the **base station** runs the v0.2 consensus engine and publishes the event
  (task 0094 owns that side, and `docs/wildfire/radio-protocol-v1.md` the wire);
* the **backend** persists it and serves it to the app, which only displays and
  acknowledges.

This module is the seam. It takes the base station's event JSON and produces
both the wire fields (stored as they arrive) and contract v1's display fields
(`docs/wildfire/app-api-contract-v1-2026-10-02.md`), which is what
`GET /v1/alerts` returns and what `AlertItem` renders. Neither vocabulary can
be derived from the other without inventing detail, so both are kept.

Strictness matches the telemetry path deliberately: an event that cannot be
trusted is not stored at all, and every rejection carries a machine-readable
reason the worker logs.
"""

from __future__ import annotations

import json
import math
from dataclasses import dataclass, field

from .validation import parse_timestamp

#: topic namespace, `+` is the publishing base station's node id
EVENT_TOPIC_TEMPLATE = "nordtronics/wildfire/+/events"

#: the four event kinds contract 0095 fixes, and what each one means to a screen
EVENT_KINDS: dict[str, tuple[str, str, str]] = {
    # event            -> (type,   severity,  state)
    "watch_raised":    ("watch", "watch", "active"),
    "watch_cleared":   ("watch", "watch", "cleared"),
    "alert_raised":    ("smoke", "warning", "active"),
    "alert_cleared":   ("smoke", "warning", "cleared"),
}

#: `node_id` may alternatively be this literal, meaning a multi-node alert
NETWORK_NODE = "network"

#: plausible ranges for the optional numeric fields (same spirit as telemetry:
#: catch unit mix-ups and broken sensors, not unusual weather)
PM25_RANGE = (0.0, 3000.0)
BASELINE_RANGE = (0.0, 3000.0)
WINDOW_MIN_RANGE = (1, 1440)


@dataclass
class EventResult:
    """Outcome of validating one events payload."""

    ok: bool
    event: dict | None = None
    errors: list[str] = field(default_factory=list)
    #: unknown keys, surfaced for logging but never fatal
    ignored_fields: list[str] = field(default_factory=list)

    @property
    def summary(self) -> str:
        return "; ".join(self.errors) if self.errors else "ok"


def _number(name: str, value: object, bounds: tuple[float, float], errors: list[str]) -> float | None:
    """Coerce and range-check one optional numeric field."""
    if isinstance(value, bool) or value is None:
        errors.append(f"{name}: expected a number, got {type(value).__name__}")
        return None
    if isinstance(value, str):
        try:
            value = float(value.strip())
        except ValueError:
            errors.append(f"{name}: expected a number, got {value!r}")
            return None
    if not isinstance(value, (int, float)):
        errors.append(f"{name}: expected a number, got {type(value).__name__}")
        return None
    number = float(value)
    low, high = bounds
    if math.isnan(number) or math.isinf(number) or number < low or number > high:
        errors.append(f"{name}: {number} outside plausible range [{low}, {high}]")
        return None
    return number


def alert_id_for(node_id: str, event: str, observed_utc: str) -> str:
    """The deterministic alert id: `ev-<node>-<event>-<instant>`.

    Deterministic on purpose. The bucket is QoS-1, so the broker may redeliver
    the same event after a reconnect; a time-based or random id would store it
    twice and show the user two alerts for one fire. `observed_utc` is the base
    station's own instant, so a genuine second event at the same second for the
    same node and kind is indistinguishable — and should be.
    """
    instant = observed_utc.replace("-", "").replace(":", "")
    return f"ev-{node_id}-{event}-{instant}"


def _title_detail(event: str, node_id: str, pm25: float | None, baseline: float | None,
                  nodes: list[str], window_min: int | None) -> tuple[str, str]:
    """The two human lines the app's alert card shows."""
    where = "the network" if node_id == NETWORK_NODE else node_id
    reading = f"PM2.5 {pm25:.1f} µg/m³" if pm25 is not None else "PM2.5 elevated"
    base = f"{baseline:.1f} µg/m³" if baseline is not None else "unknown"
    window = f"{window_min} min" if window_min is not None else "the correlation window"
    others = [n for n in nodes if n != node_id]

    if event == "watch_raised":
        return (
            f"{reading} on {where}, above its frozen baseline.",
            f"Baseline {base}. One node only, so this is a Watch — it escalates to an "
            f"Alert only if a second node confirms inside {window}.",
        )
    if event == "alert_raised":
        confirming = ", ".join(nodes) if nodes else where
        return (
            f"{reading} confirmed on {len(nodes) or 2} nodes.",
            f"Confirming nodes: {confirming}. Frozen baselines around {base}; "
            f"both inside the {window} correlation window.",
        )
    if event == "watch_cleared":
        tail = f" Other nodes still reporting: {', '.join(others)}." if others else ""
        return (
            f"Watch on {where} cleared.",
            f"{where} fell back to its threshold for the full clear period "
            f"(baseline {base}).{tail}",
        )
    # alert_cleared
    return (
        f"Alert cleared for {where}.",
        f"No node is above its limit any more (baseline {base}). The engine re-arms "
        f"after its cooldown.",
    )


def validate_event(raw: bytes | str) -> EventResult:
    """Validate one events payload.

    Required: `event` (one of the four kinds) and `observed_utc`. Optional:
    `node_id` (defaults to `network`), `pm25`, `baseline`, `nodes`,
    `window_min`. Timestamps accept the same spellings as telemetry.
    """
    if isinstance(raw, (bytes, bytearray)):
        try:
            raw = bytes(raw).decode("utf-8")
        except UnicodeDecodeError:
            return EventResult(ok=False, errors=["payload is not valid UTF-8"])

    try:
        document = json.loads(raw)
    except (json.JSONDecodeError, TypeError) as exc:
        return EventResult(ok=False, errors=[f"payload is not valid JSON: {exc}"])

    if not isinstance(document, dict):
        return EventResult(
            ok=False, errors=[f"payload must be a JSON object, got {type(document).__name__}"]
        )

    errors: list[str] = []

    event = document.get("event")
    if not isinstance(event, str):
        errors.append("event: missing or not a string")
        event = None
    elif event not in EVENT_KINDS:
        errors.append(
            "event: must be one of " + ", ".join(sorted(EVENT_KINDS)) + f", got {event!r}"
        )
        event = None

    observed_utc = None
    if "observed_utc" not in document:
        errors.append("observed_utc: missing")
    else:
        observed_utc = parse_timestamp(document["observed_utc"])
        if observed_utc is None:
            errors.append(f"observed_utc: not a usable timestamp ({document['observed_utc']!r})")

    node_id = document.get("node_id", NETWORK_NODE)
    if node_id is None:
        node_id = NETWORK_NODE
    if not isinstance(node_id, str) or not node_id.strip():
        errors.append("node_id: expected a non-empty string")
        node_id = NETWORK_NODE

    pm25 = baseline = None
    if "pm25" in document:
        pm25 = _number("pm25", document["pm25"], PM25_RANGE, errors)
    if "baseline" in document:
        baseline = _number("baseline", document["baseline"], BASELINE_RANGE, errors)

    window_min = None
    if "window_min" in document:
        raw_window = _number("window_min", document["window_min"], (0.0, 1e6), errors)
        if raw_window is not None:
            if raw_window < WINDOW_MIN_RANGE[0] or raw_window > WINDOW_MIN_RANGE[1]:
                errors.append(
                    f"window_min: {raw_window} outside plausible range "
                    f"[{WINDOW_MIN_RANGE[0]}, {WINDOW_MIN_RANGE[1]}]"
                )
            else:
                window_min = int(raw_window)

    nodes: list[str] = []
    if "nodes" in document:
        raw_nodes = document["nodes"]
        if not isinstance(raw_nodes, list) or any(not isinstance(n, str) for n in raw_nodes):
            errors.append("nodes: expected a list of node id strings")
        else:
            nodes = raw_nodes

    known = {"node_id", "event", "observed_utc", "pm25", "baseline", "nodes", "window_min"}
    ignored = sorted(k for k in document if k not in known)

    if errors:
        return EventResult(ok=False, errors=errors, ignored_fields=ignored)

    assert event is not None and observed_utc is not None
    event_type, severity, state = EVENT_KINDS[event]
    title, detail = _title_detail(event, node_id, pm25, baseline, nodes, window_min)
    return EventResult(
        ok=True,
        ignored_fields=ignored,
        event={
            "alert_id": alert_id_for(node_id, event, observed_utc),
            "node_id": node_id,
            "event": event,
            "type": event_type,
            "severity": severity,
            "state": state,
            "title": title,
            "detail": detail,
            "pm25": pm25,
            "baseline": baseline,
            "window_min": window_min,
            "nodes": nodes,
            "observed_utc": observed_utc,
        },
    )
