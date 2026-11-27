"""0268 Serve-path proof: the same checks as proof.py, over the public HTTPS Serve URL.

Run: /usr/bin/python3 mailbox/files/0268/serve_proof.py
The Serve mount is at /chese-move (root-created, see notes); both the mount root and
the /chess-move application path are exercised.
"""
import json, time, urllib.request, urllib.error, subprocess, sys
import chess

BASE = "https://home.tail2bff3f.ts.net/chese-move"
CASES = [
    ("start position", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", []),
    ("italian midgame", "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 4",
     ["e2e4", "e7e5", "f1c4", "b8c6", "g1f3"]),
    ("kingside attack", "r1bq1rk1/ppp2ppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPPB1PPP/R2Q1RK1 w - - 0 8", []),
    ("fool's mate available (punish a blunder)",
     "rnbqkbnr/pppp1ppp/8/4p3/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq - 0 2", ["f2f3", "e7e5", "g2g4"]),
]

print("== URL", BASE)
print("== GET /health (opaque since the review pass: {\"ok\": true} only)")
t = time.time()
body = urllib.request.urlopen(BASE + "/health", timeout=8).read().decode()
print("   %s  (%.2fs)" % (body.strip(), time.time() - t))

print("\n== OPTIONS preflight (Origin https://nordtronics.io)")
req = urllib.request.Request(BASE + "/chess-move", method="OPTIONS", headers={
    "Origin": "https://nordtronics.io",
    "Access-Control-Request-Method": "POST",
    "Access-Control-Request-Headers": "content-type"})
r = urllib.request.urlopen(req, timeout=8)
print("   status", r.status, {k: v for k, v in r.headers.items() if k.lower().startswith("access-control")})

print("\n== POST %s/chess-move (path through the Serve mount)" % BASE)
ok_all = True
for name, fen, moves in CASES:
    data = json.dumps({"fen": fen, "moves": moves}).encode()
    req = urllib.request.Request(BASE + "/chess-move", data=data, method="POST", headers={
        "Content-Type": "application/json", "Origin": "https://nordtronics.io"})
    t = time.time()
    resp = json.load(urllib.request.urlopen(req, timeout=8))
    el = time.time() - t
    board = chess.Board(fen)
    legal = resp["move"] in {m.uci() for m in board.legal_moves}
    print("   %-28s -> %s in %.2fs  legal=%s  within6s=%s"
          % (name, json.dumps(resp), el, legal, el < 6.0))
    ok_all = ok_all and legal and el < 6.0

print("\n== POST %s (mount root, the exact shape the page sends)" % BASE)
data = json.dumps({"fen": CASES[0][1], "moves": []}).encode()
req = urllib.request.Request(BASE, data=data, method="POST", headers={
    "Content-Type": "application/json", "Origin": "https://nordtronics.io"})
t = time.time()
resp = json.load(urllib.request.urlopen(req, timeout=8))
el = time.time() - t
print("   -> %s in %.2fs  within6s=%s" % (json.dumps(resp), el, el < 6.0))
ok_all = ok_all and el < 6.0

print("\n== fail-closed cases over Serve")
for label, payload in [("invalid fen", {"fen": "not-a-fen"}),
                       ("missing fen", {}),
                       ("oversized moves", {"fen": CASES[0][1], "moves": ["e2e4"] * 600})]:
    req = urllib.request.Request(BASE + "/chess-move", data=json.dumps(payload).encode(),
                                 method="POST", headers={"Content-Type": "application/json"})
    try:
        urllib.request.urlopen(req, timeout=8)
        print("   %s -> 200 (UNEXPECTED)" % label)
        ok_all = False
    except urllib.error.HTTPError as e:
        print("   %-16s -> %d %s" % (label, e.code, e.read().decode()[:80]))

print("\n== TLS certificate seen by the client")
print(subprocess.run(["bash", "-c",
    "echo | openssl s_client -connect home.tail2bff3f.ts.net:443 -servername home.tail2bff3f.ts.net 2>/dev/null "
    "| openssl x509 -noout -subject -issuer -dates"], capture_output=True, text=True).stdout.strip())

print("\n== journal (last 8)")
print(subprocess.run(["journalctl", "--user", "-u", "chess-bridge.service", "-n", "8", "--no-pager"],
                     capture_output=True, text=True).stdout)

print("\nRESULT: %s" % ("PASS" if ok_all else "FAIL"))
sys.exit(0 if ok_all else 1)
