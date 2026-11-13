---
task_id: '0181'
protocol_version: 1.2.0
status: verified
iteration: 1
expect-reply-within: 4h
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
proof: []
notes: |
  Host-only read-only diagnosis — no branch, no CI run, no repo file changed; proof is
  the pasted terminal output in the body, per the task's own "Proof" section.

  VERDICT: phone-side. The Home Assistant Pi and its Tailscale add-on are healthy —
  the laptop answers HA with HTTP 200 over BOTH the tailnet IP and the LAN, and
  `tailscale ping` to 100.76.58.105 returns `pong ... direct` in 2 ms. The phone
  (stephens-a35, 100.103.159.65) has been OFFLINE on the tailnet since
  2026-11-13T13:37:27Z = 06:37 MST, 3 h 39 m before this run, with rx/tx counters of
  0/0 — it is not that HA stopped answering the phone; the phone's tailnet path
  stopped existing. That produces exactly the reported symptom, because the app's
  external URL is the tailnet IP (http://100.76.58.105:8123), so with Tailscale down
  on the phone there is nothing to fall back to even when the phone is at home on
  the same Wi-Fi.

  Read-only as instructed: nothing restarted, no config touched, n8n/Docker not
  modified. One read that was attempted and could not be completed: `docker ps`
  requires root and `sudo -n` refused ("a password is required"), so the n8n
  container's network mode was NOT inspected. What I can state from the host side is
  that n8n itself is up (HTTP 200 on 127.0.0.1:5678) and that the host reaches HA on
  both paths — whether the container shares the host's tailnet route is unverified.

  Correction/nuance worth recording: `homeassistant.local` resolves to two addresses
  from this laptop (192.168.1.104 via mDNS/`getent`, 192.168.1.132 via the ping
  resolver) and both are the same MAC (2c:cf:67:16:cd:07, the Pi) and both answer
  200 — a stale/duplicate LAN address on the Pi, harmless here but it means any
  `.local`-based URL has a coin-flip resolution. The app uses the tailnet IP, so it
  is not implicated in this fault.
---

# 0181 — Diagnosis: Home Assistant unreachable from Stephen's phone

## Verdict

**The broken layer is phone-side — Stephen's phone is not on the tailnet.**
Home Assistant is up and answering; the phone's Tailscale connection is down.

- `stephens-a35` (100.103.159.65): `online: false`, `lastSeen 2026-11-13T13:37:27Z`
  = **06:37 MST**, i.e. 3 h 39 m before this run, `rx/tx: 0 / 0` (no bytes exchanged
  over the tailnet at all).
- `homeassistant` (100.76.58.105): `online: true`, `active: true`, last handshake
  10:15:29 MST (this run), direct path.
- Laptop → HA: `HTTP 200` over the tailnet in 75 ms, `HTTP 200` on the LAN in 38 ms.

So the Pi, HAOS and the Tailscale add-on are all fine, and the laptop is fine. What
fails is the one device that cannot be probed from here: the phone.

### Why that alone explains "Connection timed out"

The app's external URL is the tailnet IP (`http://100.76.58.105:8123`). A 100.x
address is only routable through the phone's Tailscale tunnel; with the VPN client
stopped there is no path at all — not even on the home Wi-Fi — and Android reports a
plain connect timeout rather than a DNS or auth error. An app pointed at the
**internal** LAN URL would have worked last night from the sofa, which matches the
timeline (it worked at bedtime, failed this morning).

### Most likely causes on the phone, in order

1. **Tailscale app was stopped / "Always-on VPN" got switched off**, or the app was
   killed by battery optimisation (very common on Samsung A-series: Settings →
   Apps → Tailscale → Battery → *Unrestricted*). Last-seen 06:37 fits a phone that
   dozed overnight and never re-established the tunnel.
2. **Phone is on a network that blocks the tunnel** (venue Wi-Fi / captive portal).
   Stephen is out at an appointment, so this is plausible for the *current* failure
   but not for the 09:47 failure if he was still home.
3. Tailscale login/session expired on the phone (key expiry is 2027-05-10, so not
   this).

Nothing here needs a Pi-side fix. Stephen's next steps are phone-side: open the
Tailscale app, confirm it says Connected and lists `homeassistant`, then retry the
HA app.

### Durable improvement for later (not applied — diagnosis only)

Setting the companion app's **internal URL** to `http://homeassistant.local:8123`
(or `http://192.168.1.132:8123`) makes the app work at home regardless of the
tailnet, and the external URL keeps working away from home. Note the `.local`
duplicate-address quirk in `notes:` — the bare IP `192.168.1.132` is the safer of
the two internal choices right now.

### On the n8n question

The laptop *can* reach HA right now (200 over the tailnet and over the LAN), so the
workflow's Home Assistant node has a working target from the host. Whether the n8n
**container** can reach `100.76.58.105` was not verified — `docker` needs root here
and I did not escalate. n8n itself is up (HTTP 200, listening on 127.0.0.1:5678).
One command settles it when Stephen is back: `docker exec <n8n> curl -s -o /dev/null
-w '%{http_code}' http://100.76.58.105:8123/`.

## Raw output

### Step 1 — `tailscale status`

```
100.78.85.106   home           nordlus@  linux    -
100.76.58.105   homeassistant  nordlus@  linux    -
100.103.159.65  stephens-a35   nordlus@  android  offline, last seen 3h ago
```

`tailscale status --json` (peer detail, abridged to the fields that matter):

```
Self: host=home  ips=[100.78.85.106]  Online=True  BackendState=Running
peer homeassistant   ips=[100.76.58.105]  online=True   active=True
     lastHandshake=2026-11-13T10:15:29.944860467-07:00  curAddr=192.168.1.104:57447
     rx/tx=10012/1068  keyExpiry=2027-05-11T09:46:00Z
peer stephens-a35    ips=[100.103.159.65] online=False  active=False
     lastSeen=2026-11-13T13:37:27.1Z  lastHandshake=0001-01-01T00:00:00Z
     rx/tx=0/0  curAddr=""  keyExpiry=2027-05-10T16:34:45Z
```

`tailscale version`: `1.104.7`

### Step 2 — tailnet reachability from the laptop

```
$ tailscale ping -c 3 100.76.58.105
pong from homeassistant (100.76.58.105) via 192.168.1.104:57447 in 2ms

$ curl -sS -o /dev/null -w 'http_code=%{http_code} time=%{time_total}' --max-time 10 \
      http://100.76.58.105:8123/
http_code=200 time=0.075167

$ curl -sS -D - -o /dev/null --max-time 8 http://100.76.58.105:8123/
HTTP/1.1 200 OK
Content-Type: text/html; charset=utf-8
Referrer-Policy: no-referrer
X-Content-Type-Options: nosniff
Server: 
X-Frame-Options: SAMEORIGIN
Content-Length: 8608
Date: Fri, 13 Nov 2026 17:15:52 GMT
```

### Step 3 — LAN reachability from the laptop

```
$ ip -4 -o addr show scope global
wlo1        192.168.1.65/24
tailscale0  100.78.85.106/32
docker0     172.17.0.1/16

$ ping -c 3 homeassistant.local
PING homeassistant.local (192.168.1.132) 56(84) bytes of data.
64 bytes from homeassistant (192.168.1.132): icmp_seq=1 ttl=64 time=2.34 ms
64 bytes from homeassistant (192.168.1.132): icmp_seq=2 ttl=64 time=1.58 ms
64 bytes from homeassistant (192.168.1.132): icmp_seq=3 ttl=64 time=2.31 ms
--- homeassistant.local ping statistics ---
3 packets transmitted, 3 received, 0% packet loss

$ getent hosts homeassistant.local
192.168.1.104   homeassistant.local

$ curl -sS -o /dev/null -w 'http_code=%{http_code} ip=%{remote_ip}' --max-time 10 \
      http://homeassistant.local:8123/
http_code=200 ip=192.168.1.104 time=0.038109

$ curl -sS -o /dev/null -w 'http_code=%{http_code} ip=%{remote_ip}' --max-time 8 \
      http://192.168.1.132:8123/
http_code=200 ip=192.168.1.132 time=0.005043
```

(LAN neighbour table confirms both addresses are the Pi, same MAC
`2c:cf:67:16:cd:07`.)

### Step 4 — reading the three results together

| Path | Result | Meaning |
|---|---|---|
| Laptop → HA over tailnet | 200 OK, 75 ms, direct | Pi + HAOS + Tailscale add-on healthy |
| Laptop → HA over LAN | 200 OK, 38 ms, both addresses | HA frontend serving on 8123 |
| Phone → HA over tailnet | phone peer offline 3 h 39 m, rx/tx 0 | **phone-side fault** |

Per the task's decision table this is the "both fine from the laptop → fault is
phone-side (the phone's Tailscale app/VPN)" case.

### n8n / Docker (read-only, not modified)

```
$ ss -ltnp | grep -E ':(5678|8123)'
LISTEN 0 4096 127.0.0.1:5678 0.0.0.0:*

$ curl -sS -o /dev/null -w 'n8n http_code=%{http_code}' --max-time 6 http://localhost:5678/
n8n http_code=200

$ sudo -n docker ps
sudo: a password is required
```
