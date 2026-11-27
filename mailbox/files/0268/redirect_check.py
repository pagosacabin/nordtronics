"""Finding 14 check: the upstream call must not follow a redirect.

Starts two stubs (8777 answers 302 -> 8778; 8778 records any hit), then a
scratch chess-bridge on 8776 whose CHESS_BRIDGE_URL points at 8777 (loopback
http, so the startup allowlist accepts it) and POSTs a position. A followed
redirect would land on 8778 carrying the bearer token; the expected result is
that 8778 is never reached and the bridge falls back to Stockfish.
"""
import json
import os
import subprocess
import sys
import threading
import time
import urllib.request
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

HITS = []


class Redirector(BaseHTTPRequestHandler):
    def do_POST(self):
        self.send_response(302)
        self.send_header("Location", "http://127.0.0.1:8778/v1/chat/completions")
        self.send_header("Content-Length", "0")
        self.end_headers()

    def log_message(self, *a):
        return


class Sink(BaseHTTPRequestHandler):
    def do_POST(self):
        HITS.append(dict(self.headers))
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", "2")
        self.end_headers()
        self.wfile.write(b"{}")

    def log_message(self, *a):
        return


def serve(port, handler):
    srv = ThreadingHTTPServer(("127.0.0.1", port), handler)
    threading.Thread(target=srv.serve_forever, daemon=True).start()
    return srv


serve(8777, Redirector)
serve(8778, Sink)

env = dict(os.environ)
env["CHESS_BRIDGE_PORT"] = "8776"
env["CHESS_BRIDGE_URL"] = "http://127.0.0.1:8777/v1/chat/completions"
env["CHESS_BRIDGE_STOCKFISH"] = "/home/astroboy/.local/bin/stockfish"
env["DEEPSEEK_API_KEY"] = "test-key-not-real"
proc = subprocess.Popen([os.path.expanduser("~/.local/share/chess-bridge/venv/bin/python"),
                         "server.py"], cwd="/home/astroboy/wt-0268/chess-bridge",
                        env=env, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
time.sleep(2.5)
try:
    body = json.dumps({"fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                       "moves": []}).encode()
    req = urllib.request.Request("http://127.0.0.1:8776/chess-move", data=body, method="POST",
                                 headers={"Content-Type": "application/json"})
    t = time.time()
    with urllib.request.urlopen(req, timeout=8) as resp:
        answer = json.loads(resp.read().decode())
    elapsed = time.time() - t
finally:
    proc.terminate()
    out = proc.communicate(timeout=10)[0]

print("answer:", json.dumps(answer), "in %.2fs" % elapsed)
print("sink hits (must be 0):", len(HITS))
print("journal tail:")
print("\n".join(out.strip().splitlines()[-4:]))
ok = len(HITS) == 0 and "move" in answer and elapsed < 6.0 and "model HTTP 302" in out
print("\nRESULT:", "PASS" if ok else "FAIL")
sys.exit(0 if ok else 1)
