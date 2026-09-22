# 0035 evidence — Home Assistant `light.*` entities (live read)

Captured: 2026-09-22T13:30 MDT (19:30 UTC) on Stephen's home network host.

## Method

The task asked for one authenticated read-only `GET /api/states`. Two facts about
this environment determine how the read had to be performed:

1. `HASS_TOKEN` is **not** exported into shell subprocesses here — the terminal and
   code-execution tools receive a scrubbed environment (`HASS_TOKEN` unset, length 0).
   A literal `curl` from a shell therefore cannot authenticate.
2. The Hermes Home Assistant integration (`ha_list_entities` / `ha_get_state`) holds the
   real `HASS_TOKEN` in its secret scope and performs the authenticated
   `GET /api/states` itself.

So the read was performed through that integration — the same token, the same endpoint,
read-only, no token pasted anywhere, no new token created.

Command equivalent (token redacted):

```
curl -s -H "Authorization: Bearer $HASS_TOKEN" \
     -H "Content-Type: application/json" \
     http://homeassistant.local:8123/api/states
```

## HTTP status

- Authenticated read (via the HA integration): **200** — the response body was returned
  and parsed (entity total and `light.*` subset below).
- Unauthenticated control probe from a shell, token deliberately absent:
  `GET http://homeassistant.local:8123/api/states` → **401 Unauthorized**, which confirms
  both that the host is reachable at `http://homeassistant.local:8123` and that the
  shell really has no token to leak.

## Result

`light.*` entities: **3**

| entity_id | friendly_name | state |
|---|---|---|
| `light.entry_light` | Entry Light | on (brightness 237) |
| `light.smart_dimmer_switch_light_1` | Light 1 | unavailable |
| `light.wled` | *(no `friendly_name` attribute)* | unavailable |

The machine-readable capture of the response objects is `evidence/0035-ha-lights.json`.

## Scope

Read-only: `GET /api/states` only. No service call, no state change, nothing created or
toggled in Home Assistant.
