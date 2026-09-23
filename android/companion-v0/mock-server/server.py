#!/usr/bin/env python3
"""Companion v0 mock backend — the exact API shapes the real VPS will serve.

Local-only by design: it refuses to bind anything but a loopback address, so the
mock can never be reachable from the LAN or the internet.

Endpoints
    GET  /api/nodes             -> [{id, pm25, temp_c, humidity, battery_v, last_seen}, ...]
    GET  /api/alerts            -> [{id, node_id, type, message, at}, ...]
    POST /api/nodes/{id}/ping   -> {"ok": true, "node_id": "<id>"}

Run (from the repository root):

    python3 android/companion-v0/mock-server/server.py --port 8000 \\
        2>&1 | tee android/companion-v0/mock-server/ping-test.log

The Android app reaches it at http://10.0.2.2:8000 (the emulator's alias for the
host loopback interface) — see app/build.gradle, the single API_BASE_URL field.
"""

import argparse
import json
import sys
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

# The emulator's alias for the host loopback interface.
LOOPBACK_HOSTS = {"127.0.0.1", "localhost", "::1"}

NODES = [
    {
        "id": "node-01",
        "pm25": 12.4,
        "temp_c": 21.3,
        "humidity": 38,
        "battery_v": 4.05,
        "last_seen": "2026-09-23T14:00:00Z",
    },
    {
        "id": "node-02",
        "pm25": 47.9,
        "temp_c": 27.8,
        "humidity": 22,
        "battery_v": 3.71,
        "last_seen": "2026-09-23T13:59:30Z",
    },
]

ALERTS = [
    {
        "id": "a1",
        "node_id": "node-01",
        "type": "smoke",
        "message": "PM2.5 rising fast",
        "at": "2026-09-23T13:58:00Z",
    },
    {
        "id": "a2",
        "node_id": "node-02",
        "type": "battery",
        "message": "Battery below 3.8 V",
        "at": "2026-09-23T12:41:00Z",
    },
    {
        "id": "a3",
        "node_id": "node-02",
        "type": "heat",
        "message": "Temperature above 27 °C",
        "at": "2026-09-23T11:06:00Z",
    },
]


def now_iso():
    return datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")


def log(line):
    """Timestamped line on stdout — captured into ping-test.log by the tee above."""
    print(f"{now_iso()} {line}", flush=True)


class Handler(BaseHTTPRequestHandler):
    server_version = "CompanionMock/0.1"
    protocol_version = "HTTP/1.1"

    def _send_json(self, code, payload):
        body = json.dumps(payload).encode("utf-8")
        self.send_response(code)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)
        log(f'-> {code} {self.path} ({len(body)} bytes) body={body.decode("utf-8")}')

    def do_GET(self):
        path = self.path.split("?", 1)[0].rstrip("/") or "/"
        log(f"<- GET {path} from {self.client_address[0]}")
        if path == "/api/nodes":
            self._send_json(200, NODES)
        elif path == "/api/alerts":
            self._send_json(200, ALERTS)
        else:
            self._send_json(404, {"error": "no such endpoint", "path": path})

    def do_POST(self):
        path = self.path.split("?", 1)[0].rstrip("/")
        length = int(self.headers.get("Content-Length") or 0)
        raw = self.rfile.read(length).decode("utf-8") if length else ""
        log(f"<- POST {path} from {self.client_address[0]} body={raw or '(empty)'}")
        parts = [p for p in path.split("/") if p]
        if len(parts) == 4 and parts[:2] == ["api", "nodes"] and parts[3] == "ping":
            node_id = parts[2]
            if any(n["id"] == node_id for n in NODES):
                self._send_json(200, {"ok": True, "node_id": node_id})
            else:
                self._send_json(404, {"ok": False, "error": "unknown node", "node_id": node_id})
        else:
            self._send_json(404, {"error": "no such endpoint", "path": path})

    def log_message(self, format, *args):
        # Default access log, kept so the log has one line per request/status.
        log("access: " + (format % args))


def main():
    ap = argparse.ArgumentParser(description="Companion v0 mock backend (localhost only)")
    ap.add_argument("--host", default="127.0.0.1", help="loopback address (default 127.0.0.1)")
    ap.add_argument("--port", type=int, default=8000)
    args = ap.parse_args()

    if args.host not in LOOPBACK_HOSTS:
        sys.exit(f"refusing to bind {args.host}: the mock backend is loopback-only by design")

    httpd = ThreadingHTTPServer((args.host, args.port), Handler)
    log(f"mock backend listening on http://{args.host}:{args.port} (loopback only)")
    log("endpoints: GET /api/nodes, GET /api/alerts, POST /api/nodes/{id}/ping")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        log("shutting down")
        httpd.server_close()


if __name__ == "__main__":
    main()
