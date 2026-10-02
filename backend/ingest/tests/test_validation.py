"""Payload validation tests."""

from __future__ import annotations

import math

import pytest

from ingest.validation import parse_timestamp, validate_payload

GOOD = {
    "node_id": "node-01",
    "pm25": 12.3,
    "temperature_c": 18.5,
    "humidity_pct": 42.0,
    "battery_v": 3.92,
}


def payload(**overrides) -> bytes:
    import json

    document = dict(GOOD)
    document.update(overrides)
    for key, value in list(document.items()):
        if value is _MISSING:
            document.pop(key)
    return json.dumps(document).encode()


class _Missing:
    pass


_MISSING = _Missing()


def test_valid_payload_is_accepted():
    result = validate_payload(payload(observed_utc="2026-09-24T23:00:00Z"))
    assert result.ok, result.errors
    assert result.reading == {
        "pm25": 12.3,
        "temperature_c": 18.5,
        "humidity_pct": 42.0,
        "battery_v": 3.92,
        "observed_utc": "2026-09-24T23:00:00Z",
    }
    assert result.claimed_node_id == "node-01"
    assert result.ignored_fields == []


def test_reading_without_timestamp_is_accepted():
    result = validate_payload(payload())
    assert result.ok, result.errors
    assert result.reading["observed_utc"] is None


@pytest.mark.parametrize("field", ["pm25", "temperature_c", "humidity_pct", "battery_v"])
def test_missing_required_field_is_rejected(field):
    result = validate_payload(payload(**{field: _MISSING}))
    assert not result.ok
    assert f"{field}: missing" in result.errors


@pytest.mark.parametrize(
    "field,value",
    [
        ("pm25", -1.0),
        ("pm25", 5000.0),
        ("temperature_c", -80.0),
        ("temperature_c", 120.0),
        ("humidity_pct", 101.0),
        ("humidity_pct", -0.1),
        ("battery_v", 48.0),
    ],
)
def test_out_of_range_is_rejected(field, value):
    result = validate_payload(payload(**{field: value}))
    assert not result.ok
    assert any("outside plausible range" in error for error in result.errors)


@pytest.mark.parametrize("value", ["warm", True, None, [], {}])
def test_non_numeric_is_rejected(value):
    result = validate_payload(payload(temperature_c=value))
    assert not result.ok
    assert any(error.startswith("temperature_c:") for error in result.errors)


def test_numeric_string_is_coerced():
    result = validate_payload(payload(pm25="12.5"))
    assert result.ok, result.errors
    assert result.reading["pm25"] == 12.5


def test_nan_and_infinity_are_rejected():
    for literal in ("NaN", "Infinity", "-Infinity"):
        raw = b'{"pm25": ' + literal.encode() + b', "temperature_c": 18.5, "humidity_pct": 42.0, "battery_v": 3.92}'
        result = validate_payload(raw)
        assert not result.ok, literal
        assert any("not a finite number" in error for error in result.errors)


def test_boundary_values_are_accepted():
    result = validate_payload(payload(pm25=0.0, humidity_pct=0.0, battery_v=0.0, temperature_c=-60.0))
    assert result.ok, result.errors


def test_values_are_rounded_to_four_places():
    result = validate_payload(payload(pm25=12.3456789))
    assert result.ok
    assert result.reading["pm25"] == 12.3457


def test_malformed_json_is_rejected():
    result = validate_payload(b"pm25=12.3")
    assert not result.ok
    assert "not valid JSON" in result.errors[0]


def test_non_object_json_is_rejected():
    result = validate_payload(b"[1, 2, 3]")
    assert not result.ok
    assert "must be a JSON object" in result.errors[0]


def test_non_utf8_payload_is_rejected():
    result = validate_payload(b"\xff\xfe\x00")
    assert not result.ok
    assert "not valid UTF-8" in result.errors[0]


def test_unknown_fields_are_ignored_not_fatal():
    result = validate_payload(payload(voc_index=42, firmware="1.2.3"))
    assert result.ok, result.errors
    assert result.ignored_fields == ["firmware", "voc_index"]


def test_non_string_node_id_is_rejected():
    result = validate_payload(payload(node_id=7))
    assert not result.ok
    assert "node_id: expected a string" in result.errors


def test_string_payload_is_accepted():
    result = validate_payload(payload().decode())
    assert result.ok, result.errors


def test_empty_payload_is_rejected():
    result = validate_payload(b"")
    assert not result.ok


def test_multiple_faults_are_all_reported():
    raw = b'{"pm25": "nope", "temperature_c": 200.0}'
    result = validate_payload(raw)
    assert not result.ok
    assert len(result.errors) >= 4  # two bad fields plus two missing
    assert result.summary != "ok"


@pytest.mark.parametrize(
    "value,expected",
    [
        ("2026-09-24T23:00:00Z", "2026-09-24T23:00:00Z"),
        ("2026-09-24T23:00:00+00:00", "2026-09-24T23:00:00Z"),
        ("2026-09-24T17:00:00-06:00", "2026-09-24T23:00:00Z"),
        ("2026-09-24T23:00:00", "2026-09-24T23:00:00Z"),
        ("2026-09-24T23:00:00.250Z", "2026-09-24T23:00:00Z"),
        (1790295600, "2026-09-25T00:20:00Z"),
        (1790295600000, "2026-09-25T00:20:00Z"),
    ],
)
def test_timestamps_are_normalised(value, expected):
    normalised = parse_timestamp(value)
    assert normalised == expected


@pytest.mark.parametrize("value", ["", "yesterday", "2026-13-45T99:00:00Z", None, True, [], {}, -1, 1e13, math.inf])
def test_unusable_timestamps_return_none(value):
    assert parse_timestamp(value) is None


def test_unusable_timestamp_is_rejected():
    result = validate_payload(payload(observed_utc="last tuesday"))
    assert not result.ok
    assert any("not a usable timestamp" in error for error in result.errors)


def test_timestamp_aliases_are_honoured():
    for alias in ("observed_utc", "ts", "timestamp"):
        result = validate_payload(payload(**{alias: "2026-09-24T23:00:00Z"}))
        assert result.ok, (alias, result.errors)
        assert result.reading["observed_utc"] == "2026-09-24T23:00:00Z"
        assert result.ignored_fields == []
