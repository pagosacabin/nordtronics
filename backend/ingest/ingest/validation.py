"""Payload validation for the wildfire telemetry topic.

Validation is deliberately strict: a reading that cannot be trusted is not
stored at all, because the whole point of the dataset is that insurers can
rely on it. Every rejection carries a machine-readable reason that the worker
logs, so a misbehaving node is visible rather than silent.
"""

from __future__ import annotations

import json
import math
from dataclasses import dataclass, field
from datetime import datetime, timezone

#: topic namespace, `+` is the node id wildcard
TOPIC_TEMPLATE = "nordtronics/wildfire/+/telemetry"

#: every field the node must send, with the physically plausible range.
#: Ranges are wide on purpose — they catch broken sensors and unit mix-ups,
#: not unusual weather.
FIELD_RANGES: dict[str, tuple[float, float]] = {
    "pm25": (0.0, 2000.0),          # ug/m3 — PMS5003 saturates around 1000
    "temperature_c": (-60.0, 85.0),  # BME680 operating range
    "humidity_pct": (0.0, 100.0),
    "battery_v": (0.0, 30.0),        # 30 V ceiling allows a 12/24 V pack
}

REQUIRED_FIELDS: tuple[str, ...] = tuple(FIELD_RANGES)

#: accepted spellings for the node's own sample time, in priority order
TIMESTAMP_FIELDS: tuple[str, ...] = ("observed_utc", "ts", "timestamp")


@dataclass
class ValidationResult:
    """Outcome of validating one MQTT payload."""

    ok: bool
    reading: dict | None = None
    errors: list[str] = field(default_factory=list)
    #: node id claimed inside the payload, if any (checked against the topic)
    claimed_node_id: str | None = None
    #: unknown keys, surfaced for logging but never fatal
    ignored_fields: list[str] = field(default_factory=list)

    @property
    def summary(self) -> str:
        return "; ".join(self.errors) if self.errors else "ok"


def _iso_z(dt: datetime) -> str:
    return dt.astimezone(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def parse_timestamp(value: object) -> str | None:
    """Normalise a node timestamp to `YYYY-MM-DDTHH:MM:SSZ`.

    Accepts an ISO-8601 string or a Unix epoch (seconds or milliseconds).
    Returns None when the value cannot be interpreted.
    """
    if isinstance(value, bool):
        return None
    if isinstance(value, (int, float)):
        epoch = float(value)
        if math.isnan(epoch) or math.isinf(epoch):
            return None
        if epoch > 1e11:  # milliseconds
            epoch /= 1000.0
        if epoch < 0 or epoch > 4102444800:  # before 1970 or after 2100
            return None
        return _iso_z(datetime.fromtimestamp(epoch, tz=timezone.utc))
    if not isinstance(value, str):
        return None

    text = value.strip()
    if not text:
        return None
    if text.endswith(("Z", "z")):
        text = text[:-1] + "+00:00"
    try:
        parsed = datetime.fromisoformat(text)
    except ValueError:
        return None
    if parsed.tzinfo is None:
        parsed = parsed.replace(tzinfo=timezone.utc)
    return _iso_z(parsed)


def _coerce_number(name: str, value: object, errors: list[str]) -> float | None:
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
    if math.isnan(number) or math.isinf(number):
        errors.append(f"{name}: not a finite number")
        return None
    low, high = FIELD_RANGES[name]
    if number < low or number > high:
        errors.append(f"{name}: {number} outside plausible range [{low}, {high}]")
        return None
    return number


def validate_payload(raw: bytes | str) -> ValidationResult:
    """Validate one telemetry payload.

    The payload is a JSON object carrying at minimum `pm25`,
    `temperature_c`, `humidity_pct` and `battery_v`. `node_id` and an optional
    sample timestamp may be included; unknown keys are ignored (the firmware
    will grow fields over time).
    """
    if isinstance(raw, (bytes, bytearray)):
        try:
            raw = bytes(raw).decode("utf-8")
        except UnicodeDecodeError:
            return ValidationResult(ok=False, errors=["payload is not valid UTF-8"])

    try:
        document = json.loads(raw)
    except (json.JSONDecodeError, TypeError) as exc:
        return ValidationResult(ok=False, errors=[f"payload is not valid JSON: {exc}"])

    if not isinstance(document, dict):
        return ValidationResult(
            ok=False, errors=[f"payload must be a JSON object, got {type(document).__name__}"]
        )

    errors: list[str] = []
    reading: dict = {}

    claimed_node_id = document.get("node_id")
    if claimed_node_id is not None and not isinstance(claimed_node_id, str):
        errors.append("node_id: expected a string")

    for name in REQUIRED_FIELDS:
        if name not in document:
            errors.append(f"{name}: missing")
            continue
        number = _coerce_number(name, document[name], errors)
        if number is not None:
            reading[name] = round(number, 4)

    observed_utc = None
    for candidate in TIMESTAMP_FIELDS:
        if candidate in document:
            observed_utc = parse_timestamp(document[candidate])
            if observed_utc is None:
                errors.append(f"{candidate}: not a usable timestamp")
            break
    reading["observed_utc"] = observed_utc

    known = set(REQUIRED_FIELDS) | {"node_id"} | set(TIMESTAMP_FIELDS)
    ignored = sorted(k for k in document if k not in known)

    if errors:
        return ValidationResult(ok=False, errors=errors, ignored_fields=ignored)
    return ValidationResult(
        ok=True,
        reading=reading,
        claimed_node_id=claimed_node_id,
        ignored_fields=ignored,
    )
