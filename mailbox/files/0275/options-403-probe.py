#!/usr/bin/env python3
"""Probe the do_OPTIONS 403 path (LOW-2 fix).

Each case uses one socket, reads the complete response (headers + the exact
Content-Length body), then sends a second identical request on the same socket:

  403 (origin not allowed) -- the socket must be closed, so the second request is
                              never answered (EOF, RST or EPIPE);
  204 (allowed origin)     -- the connection must stay alive and answer it.
"""
import os, socket, subprocess, sys, time

APP = "/home/astroboy/wt/0275-pocket-conquest/pocket-conquest-bridge"
env = dict(os.environ)
env["PC_BRIDGE_ALLOWED_ORIGINS"] = "https://good.example"
env["PC_BRIDGE_PORT"] = "8768"
env.setdefault("DEEPSEEK_API_KEY", "sk-probe-not-used")   # no model call is made here
proc = subprocess.Popen([sys.executable, APP + "/server.py"], env=env,
                        stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
time.sleep(1.5)

REQ = ("OPTIONS /pocket-conquest-move HTTP/1.1\r\nHost: x\r\n"
       "Origin: %s\r\nContent-Length: 0\r\n\r\n")


class Reader:
    def __init__(self, sock):
        self.s = sock
        self.buf = b""

    def _fill(self):
        chunk = self.s.recv(4096)
        if not chunk:
            return False
        self.buf += chunk
        return True

    def response(self):
        """Return (status_line, headers dict, body) for one complete response."""
        while b"\r\n\r\n" not in self.buf:
            if not self._fill():
                return None
        head, _, self.buf = self.buf.partition(b"\r\n\r\n")
        lines = head.decode(errors="replace").splitlines()
        headers = {}
        for line in lines[1:]:
            k, _, v = line.partition(":")
            headers[k.strip().lower()] = v.strip()
        length = int(headers.get("content-length", "0"))
        while len(self.buf) < length:
            if not self._fill():
                break
        body, self.buf = self.buf[:length], self.buf[length:]
        return lines[0], headers, body


try:
    for label, origin, want_alive in (("403 origin not allowed", "https://evil.example", False),
                                      ("204 allowed origin", "https://good.example", True)):
        s = socket.create_connection(("127.0.0.1", 8768), timeout=5)
        s.settimeout(3)
        reader = Reader(s)
        s.sendall((REQ % origin).encode())
        first = reader.response()
        conn = first[1].get("connection", "(absent)")
        alive = None
        try:
            s.sendall((REQ % origin).encode())
            second = reader.response()
            alive = second is not None
        except (socket.timeout, BrokenPipeError, ConnectionResetError):
            alive = False
        finally:
            s.close()
        verdict = "PASS" if alive == want_alive else "FAIL"
        print("%-24s %-22s Connection:%-9s body=%dB  second-request-answered=%-5s expected=%-5s %s"
              % (label, first[0], conn, len(first[2]), alive, want_alive, verdict))
finally:
    proc.terminate()
    try:
        out = proc.communicate(timeout=5)[0].decode(errors="replace")
    except subprocess.TimeoutExpired:
        proc.kill(); out = ""
    print("--- server journal ---")
    for line in out.splitlines():
        if "OPTIONS" in line or "listening" in line:
            print(line)
