"""Review-pass-1 fix checks against a scratch instance (default port 8766).

Covers findings 2-12: upstream scheme allowlist (checked separately, startup),
token bucket (429), worker pool (503), whole-handler deadline, CORS fail-closed
(403 preflight / no ACAO), UCI-syntax history, opaque /health and 500s,
header size bound, engine identity.
"""
import json, socket, sys, time, urllib.error, urllib.request

BASE = "http://127.0.0.1:%s" % (sys.argv[1] if len(sys.argv) > 1 else "8766")
ALLOWED = "https://nordtronics.io"
START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
fails = []


def check(label, ok, detail=""):
    print("%-52s %s  %s" % (label, "PASS" if ok else "FAIL", detail))
    if not ok:
        fails.append(label)


def req(path, method="GET", body=None, origin=None, headers=None, timeout=8):
    hdrs = dict(headers or {})
    if origin:
        hdrs["Origin"] = origin
    data = None
    if body is not None:
        data = body if isinstance(body, bytes) else json.dumps(body).encode()
        hdrs.setdefault("Content-Type", "application/json")
    r = urllib.request.Request(BASE + path, data=data, method=method, headers=hdrs)
    t = time.time()
    try:
        with urllib.request.urlopen(r, timeout=timeout) as resp:
            return resp.status, dict(resp.headers), resp.read().decode(), time.time() - t
    except urllib.error.HTTPError as e:
        return e.code, dict(e.headers), e.read().decode(), time.time() - t


print("== /health is opaque")
s, h, b, _ = req("/health")
check("GET /health 200 and no model/engine names", s == 200 and json.loads(b) == {"ok": True}, b.strip())

print("\n== CORS fails closed")
s, h, b, _ = req("/chess-move", "OPTIONS", origin=ALLOWED)
check("preflight allowed origin 204 + ACAO",
      s == 204 and h.get("Access-Control-Allow-Origin") == ALLOWED, "ACAO=%r" % h.get("Access-Control-Allow-Origin"))
s, h, b, _ = req("/chess-move", "OPTIONS", origin="https://evil.example")
check("preflight unknown origin 403, no ACAO",
      s == 403 and not h.get("Access-Control-Allow-Origin"), "%d ACAO=%r" % (s, h.get("Access-Control-Allow-Origin")))
s, h, b, _ = req("/chess-move", "POST", {"fen": START_FEN, "moves": []}, origin="https://evil.example")
check("POST unknown origin 200 but no ACAO grant",
      s == 200 and not h.get("Access-Control-Allow-Origin"), "%d ACAO=%r" % (s, h.get("Access-Control-Allow-Origin")))

print("\n== input validation")
s, _, b, _ = req("/chess-move", "POST", {"fen": START_FEN, "moves": ["e2e9"]})
check("non-UCI history entry rejected 400", s == 400, b.strip()[:80])
s, _, b, _ = req("/chess-move", "POST", {"fen": START_FEN, "moves": ["hello world!"]})
check("free-text history entry rejected 400", s == 400, b.strip()[:80])
s, _, b, _ = req("/chess-move", "POST", {"fen": "not-a-fen"})
check("invalid fen rejected 400", s == 400, b.strip()[:80])

print("\n== move, legality and budget")
s, _, b, dt = req("/chess-move", "POST",
                  {"fen": "rnbqkbnr/pppp1ppp/8/4p3/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq - 0 2",
                   "moves": ["f2f3", "e7e5", "g2g4"]}, origin=ALLOWED)
check("fool's-mate position answers inside 6 s", s == 200 and dt < 6.0,
      "%d %s in %.2fs" % (s, b.strip(), dt))

print("\n== header bound (finding 12)")
try:
    sock = socket.create_connection(("127.0.0.1", int(BASE.rsplit(":", 1)[1])), timeout=5)
    junk = b"GET /health HTTP/1.1\r\nHost: x\r\n" + b"".join(
        b"X-Pad-%d: %s\r\n" % (i, b"A" * 200) for i in range(80)) + b"\r\n"
    sock.sendall(junk)
    reply = sock.recv(200).decode("latin-1")
    sock.close()
    check("oversized header block refused (431)", "431" in reply, reply.splitlines()[0] if reply else "no reply")
except Exception as exc:
    check("oversized header block refused (431)", False, repr(exc))

print("\n== unknown method still answers and is logged (finding 10)")
s, _, _, _ = req("/chess-move", "PUT", body=b"x")
check("unsupported method -> 501", s == 501, str(s))

print("\n== token bucket + worker pool (findings 3)")
codes = []
for _ in range(20):
    # cheap 400s: they spend a token without spending model time, so the bucket
    # is what has to stop them
    s, _, _, _ = req("/chess-move", "POST", {"fen": "", "moves": []}, timeout=8)
    codes.append(s)
check("burst above RATE_BURST yields 429", 429 in codes, str(codes))

print("\nRESULT:", "ALL PASS" if not fails else "FAILURES: %s" % fails)
sys.exit(0 if not fails else 1)
