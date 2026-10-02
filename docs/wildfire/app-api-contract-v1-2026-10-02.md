# Wildfire Companion — API Contract v1 (2026-10-02)

The contract the app builds against **now** (via a mock) and the real base station /
backend implements **later**. Firmware-independent: no radio, no hardware needed.

Wire format: JSON, UTF-8. Units on the wire are **SI** (`µg/m³`, `°C`, `%RH`, `V`, `dBm`);
the app converts to °F for display. Times are **UTC ISO-8601**; the app formats local.
Privacy: **no coordinates on the wire, ever.** `location_label` / `zone` are user-defined
strings. Property-schematic positions are client-side layout hints only.

## Existing surface (live 2026-10-02 — do not break)

`GET /healthz` → `{"status":"ok","database":"ok","schema_version":1,"generated_utc":"..."}`

`GET /v1/nodes` → today returns:
```json
{"nodes":[{"node_id":"node-01","first_seen_utc":"2026-09-25T19:18:44Z",
"last_seen_utc":"2026-09-25T19:20:01Z","reading_count":2,"age_seconds":572138,
"status":"stale","latest":{"pm25":7.5,"temperature_c":21.0,"humidity_pct":41.0,"battery_v":3.8}}],
"count":1,"stale_after_seconds":900,"generated_utc":"2026-10-02T10:15:39Z"}
```
v1 **extends** each node object additively (all new fields optional until the backend
implements them; the mock provides all of them):
`display_name` ("Node 01"), `zone` ("Zone 7"), `location_label` ("Sierra Ridge Trail"),
`firmware` ("v2.3.1"), `hw_rev` ("HW Rev 4"), `serial` ("I-01-7A3F-22"),
`rssi_dbm` (-68), `link_margin_db` (32), `gateway_hops` (1),
`battery_pct` (87), `charging` (true), `charge_today_pct` (6).
`status` stays `live` | `stale`; server computes `age_seconds`; staleness threshold stays
`stale_after_seconds: 900`.

## New in v1

### `GET /v1/network/status` — consensus summary (property banner)
```json
{"state":"watch","nodes_reporting":2,"nodes_total":2,
 "outside_limits":["node-02"],"note":"1 of 2 nodes outside limits. Rest of network at baseline.",
 "last_packet_utc":"2026-10-02T10:11:39Z","generated_utc":"..."}
```
`state`: `healthy` | `watch` | `alert`.

### `GET /v1/nodes/{id}` — node detail
Full node object (as extended above) plus:
```json
{"trends_12h":{"pm25":[12.1,12.4,...],"temperature_c":[...],"humidity_pct":[...]}}
```
Arrays are oldest→newest, one point per 30 min (25 points for 12 h). The app draws sparklines
from these; no separate sparkline endpoint.

### `GET /v1/nodes/{id}/readings?metric=pm25&hours=24` — trend chart
```json
{"node_id":"node-01","metric":"pm25","unit":"µg/m³","hours":24,
 "points":[{"t":"2026-10-01T10:15:00Z","v":9.8},...]}
```
`metric`: `pm25` | `temperature_c` | `humidity_pct` | `battery_v`. ≤1 point per 15 min.

### `GET /v1/alerts` — alert feed (currently 404; v1 defines it)
```json
{"alerts":[
 {"alert_id":"al-004","type":"watch","node_id":"node-02","severity":"watch",
  "state":"active","title":"PM2.5 47.9 µg/m³ on one node, awaiting confirmation.",
  "detail":"Elevated on a single node. Escalates only if a second node confirms.",
  "created_utc":"2026-10-02T09:40:00Z","updated_utc":"2026-10-02T09:40:00Z"},
 {"alert_id":"al-003","type":"battery","node_id":"node-02","severity":"notice",
  "state":"active","title":"Battery below 3.8 V.","detail":"3.71 V at last report.",
  "created_utc":"2026-10-02T08:12:00Z","updated_utc":"2026-10-02T08:12:00Z"},
 {"alert_id":"al-002","type":"heat","node_id":"node-02","severity":"warning",
  "state":"acknowledged","title":"Temperature above 27 °C.","detail":"28.1 °C at last report.",
  "created_utc":"2026-10-02T07:06:00Z","updated_utc":"2026-10-02T08:00:00Z"},
 {"alert_id":"al-001","type":"smoke","node_id":"node-01","severity":"warning",
  "state":"cleared","title":"PM2.5 rising fast.","detail":"Cleared after node returned to baseline.",
  "created_utc":"2026-09-23T13:58:00Z","updated_utc":"2026-09-23T15:20:00Z"}
],"generated_utc":"..."}
```
`type`: `watch` | `smoke` | `battery` | `heat`.
`severity`: `watch` | `notice` | `warning` | `critical`.
`state`: `active` | `acknowledged` | `cleared` | `snoozed`.
Badge count rule (app computes): `active` items of any severity + `active` watches.
Never counts `acknowledged`, `cleared`, or `snoozed`.

### Alert actions
- `POST /v1/alerts/{id}/acknowledge` → state → `acknowledged`
- `POST /v1/alerts/{id}/clear` → state → `cleared`
- `POST /v1/alerts/{id}/snooze` `{"hours":24}` → state → `snoozed`, `snoozed_until_utc`
- Acknowledge is offered **only** on `active` items. Cleared/snoozed items show "View trend" only.

## Consensus rule (base station — Stephen's call 2026-10-02; never the app)

The base station runs detection. The backend persists and serves the resulting alerts;
the app is display + acknowledge only.

- One node above its PM2.5 limit → emit/keep a `watch` alert on that node. **Never auto-escalates.**
- ≥2 nearby nodes rising above limit inside the same 15-minute window → emit `smoke` alert,
  `severity: warning` (or `critical` past the high band), `state: active`.
- Watch clears automatically when the node returns under the limit for 3 consecutive windows.
- Threshold bands (display): Clean < 12 · Elevated 12–35 · High > 35 µg/m³.

## Mock mode (what the app task uses)

Until the backend implements `/v1/alerts`, `/v1/network/status`, and the extended node
fields, the app ships a `MockApi` implementing **this exact contract** and serving the
mock state in `app-ui-spec-v2-2026-10-02.md` (Node 01 healthy values, Node 02 watch values,
the four alerts above, badge = 2). `MockApi` and the real `ApiClient` share one interface;
swapping the data source later changes no screen code.

## Out of scope for v1

Real `/v1/alerts` backend implementation (later task). Push notifications. Map tiles or
GPS. Per-node alert threshold editing (v2).
