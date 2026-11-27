"""0271 Serve-path proof: chess-bridge v2 over the public HTTPS Tailscale Serve URL.

Run: /usr/bin/python3 mailbox/files/0271/serve_proof.py

The live mount is /chese-move (root-created, 0268's owed spelling item), so both the
mount root and the /chess-move application path are exercised. Every returned move is
checked legal for its FEN with python-chess and checked to be one of Stockfish's
top-N candidates, computed here independently at the same difficulty settings.
"""
import json
import time
import urllib.error
import urllib.request

import chess
import chess.engine

BASE = "https://home.tail2bff3f.ts.net/chese-move"
SF = "/home/astroboy/.local/bin/stockfish"
DIFF = {"easy": (3, 8, 0.25), "normal": (4, 12, 0.45), "hard": (5, 16, 0.80)}
HEADERS = {"Content-Type": "application/json", "Origin": "https://nordtronics.io"}

START = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
ITALIAN = "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/2N2N2/PPPP1PPP/R1BQK2R w KQkq - 6 6"
BLUNDER = "rnbqkbnr/pppp1ppp/8/4p3/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq g4 0 2"
BLUNDER_MOVES = ["f2f3", "e7e5", "g2g4"]

failures = []


def candidates_for(fen, difficulty):
    n, depth, cap = DIFF[difficulty]
    board = chess.Board(fen)
    with chess.engine.SimpleEngine.popen_uci(SF) as engine:
        engine.configure({"Threads": 1, "Hash": 16})
        infos = engine.analyse(board, chess.engine.Limit(depth=depth, time=cap),
                               multipv=min(n, board.legal_moves.count()))
    if isinstance(infos, dict):
        infos = [infos]
    return [i["pv"][0].uci() for i in infos if i.get("pv")]


def post(path, payload):
    req = urllib.request.Request(BASE + path, data=json.dumps(payload).encode(),
                                 headers=HEADERS, method="POST")
    t0 = time.time()
    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            return resp.status, json.loads(resp.read().decode()), time.time() - t0
    except urllib.error.HTTPError as exc:
        return exc.code, json.loads(exc.read().decode() or "{}"), time.time() - t0


print("== URL", BASE)
t0 = time.time()
print("   GET /health -> %s  (%.2fs)"
      % (urllib.request.urlopen(BASE + "/health", timeout=8).read().decode().strip(), time.time() - t0))

print("\n== OPTIONS preflight (Origin https://nordtronics.io)")
req = urllib.request.Request(BASE + "/chess-move", method="OPTIONS",
                             headers={"Origin": "https://nordtronics.io",
                                      "Access-Control-Request-Method": "POST",
                                      "Access-Control-Request-Headers": "content-type"})
with urllib.request.urlopen(req, timeout=8) as resp:
    print("   status %s  %s" % (resp.status, {
        k: v for k, v in resp.headers.items() if k.lower().startswith("access-control")}))

cases = [
    ("start position / normal", START, [], "normal"),
    ("italian midgame / normal", ITALIAN,
     ["e2e4", "e7e5", "g1f3", "b8c6", "f1c4", "g8f6"], "normal"),
    ("blunder (fool's mate) / normal", BLUNDER, BLUNDER_MOVES, "normal"),
    ("blunder / easy (teach me)", BLUNDER, BLUNDER_MOVES, "easy"),
    ("blunder / hard (aggressive)", BLUNDER, BLUNDER_MOVES, "hard"),
]

print("\n== POST %s/chess-move" % BASE)
for name, fen, moves, difficulty in cases:
    payload = {"fen": fen, "moves": moves}
    if difficulty != "normal":
        payload["difficulty"] = difficulty
    status, doc, elapsed = post("/chess-move", payload)
    board = chess.Board(fen)
    cands = candidates_for(fen, difficulty)
    legal = doc.get("move") in {m.uci() for m in board.legal_moves}
    in_cand = doc.get("move") in cands
    print("   %-32s -> %s  %.2fs  legal=%s in_candidates=%s"
          % (name, json.dumps(doc), elapsed, legal, in_cand))
    print("      candidates: %s" % cands)
    ok = status == 200 and legal and in_cand and elapsed < 6.0
    if name.startswith("blunder"):
        comment = (doc.get("comment") or "").lower()
        names = ("g4" in comment or "g2-g4" in comment or "g2g4" in comment)
        print("      blunder comment names the move: %s" % names)
        ok = ok and bool(comment) and names
    if not ok:
        failures.append(name)

print("\n== POST mount root (the exact shape the page sends)")
status, doc, elapsed = post("", {"fen": START, "moves": []})
print("   -> %s  %.2fs  within6s=%s" % (json.dumps(doc), elapsed, elapsed < 6.0))
if elapsed >= 6.0:
    failures.append("mount root")

print("\n== fail-closed cases over Serve")
for label, payload, want in [
        ("invalid fen", {"fen": "not-a-fen"}, 400),
        ("600 moves", {"fen": START, "moves": ["e2e4"] * 600}, 400),
        ("missing fen", {"moves": []}, 400)]:
    status, doc, elapsed = post("/chess-move", payload)
    print("   %-12s status=%s body=%s" % (label, status, doc))
    if status != want:
        failures.append(label)

print("\n== TLS")
import ssl
import socket
hostname = "home.tail2bff3f.ts.net"
ctx = ssl.create_default_context()
with socket.create_connection((hostname, 443), timeout=10) as sock:
    with ctx.wrap_socket(sock, server_hostname=hostname) as tls:
        cert = tls.getpeercert()
print("   CN=%s  issuer=%s  notAfter=%s"
      % (dict(x[0] for x in cert["subject"]).get("commonName"),
         dict(x[0] for x in cert["issuer"]).get("organizationName"),
         cert["notAfter"]))

print("\nFAILURES: %s" % (failures or "none"))
raise SystemExit(1 if failures else 0)
