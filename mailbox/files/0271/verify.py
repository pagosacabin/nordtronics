#!/usr/bin/env python3
"""Independent verification of the 0271 chess-bridge v2 build (supervisor's own run)."""
import json, os, subprocess, sys, time, urllib.request, urllib.error
import chess, chess.engine

PORT = 8766
BASE = "http://127.0.0.1:%d" % PORT
SF = "/home/astroboy/.local/bin/stockfish"
DIFF = {"easy": (3, 8, 0.25), "normal": (4, 12, 0.45), "hard": (5, 16, 0.80)}
START = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
ITALIAN = "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/2N2N2/PPPP1PPP/R1BQK2R w KQkq - 6 6"
KINGSIDE = "r1bq1rk1/ppp2ppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPP2PPP/R1BQ1RK1 w - - 0 8"
BLUNDER = "rnbqkbnr/pppp1ppp/8/4p3/6P1/5P2/PPPPP2P/RNBQKBNR b KQkq g4 0 2"
BLUNDER_MOVES = ["f2f3", "e7e5", "g2g4"]

def post(body, timeout=10):
    data = json.dumps(body).encode()
    req = urllib.request.Request(BASE + "/chess-move", data=data,
                                 headers={"Content-Type": "application/json"}, method="POST")
    t0 = time.monotonic()
    try:
        with urllib.request.urlopen(req, timeout=timeout) as r:
            return r.status, json.loads(r.read().decode()), time.monotonic() - t0
    except urllib.error.HTTPError as e:
        return e.code, json.loads(e.read().decode() or "{}"), time.monotonic() - t0

def candidates_for(fen, difficulty):
    n, depth, cap = DIFF[difficulty]
    b = chess.Board(fen)
    with chess.engine.SimpleEngine.popen_uci(SF) as e:
        e.configure({"Threads": 1, "Hash": 16})
        infos = e.analyse(b, chess.engine.Limit(depth=depth, time=cap), multipv=min(n, b.legal_moves.count()))
    if isinstance(infos, dict):
        infos = [infos]
    return [i["pv"][0].uci() for i in infos if i.get("pv")]

def main():
    env = dict(os.environ)
    with open(os.path.expanduser("~/.hermes/.env")) as f:
        for line in f:
            if line.startswith("DEEPSEEK_API_KEY="):
                env["DEEPSEEK_API_KEY"] = line.split("=", 1)[1].strip().strip('"\'')
                break
    env["CHESS_BRIDGE_PORT"] = str(PORT)
    log = open(os.path.expanduser("~/.hermes/profiles/cronrunner/cache/scratch/0271/scratch-server.log"), "w")
    proc = subprocess.Popen(["/home/astroboy/.local/share/chess-bridge/venv/bin/python",
                             "/home/astroboy/wt-0271/chess-bridge/server.py"],
                            env=env, stdout=log, stderr=subprocess.STDOUT)
    try:
        for _ in range(50):
            time.sleep(0.1)
            try:
                urllib.request.urlopen(BASE + "/health", timeout=1)
                break
            except Exception:
                continue
        cases = [
            ("startpos/normal", {"fen": START, "moves": []}, "normal"),
            ("italian/normal", {"fen": ITALIAN, "moves": ["e2e4","e7e5","g1f3","b8c6","f1c4","g8f6"]}, "normal"),
            ("kingside/normal", {"fen": KINGSIDE, "moves": ["e2e4","e7e5","g1f3","b8c6","f1c4","g8f6","e1g1","f8c5","d2d3","d7d6","b1c3","e8g8"]}, "normal"),
            ("blunder/normal", {"fen": BLUNDER, "moves": BLUNDER_MOVES}, "normal"),
            ("blunder/easy", {"fen": BLUNDER, "moves": BLUNDER_MOVES, "difficulty": "easy", "personality": "teach me"}, "easy"),
            ("blunder/hard", {"fen": BLUNDER, "moves": BLUNDER_MOVES, "difficulty": "hard", "personality": "aggressive"}, "hard"),
        ]
        failures = []
        for name, body, diff in cases:
            st, doc, el = post(body)
            cand = candidates_for(body["fen"], diff)
            legal = {m.uci() for m in chess.Board(body["fen"]).legal_moves}
            in_cand = doc.get("move") in cand
            legal_ok = doc.get("move") in legal
            print("%-16s status=%s elapsed=%.3fs move=%-5s legal=%s in_candidates=%s"
                  % (name, st, el, doc.get("move"), legal_ok, in_cand))
            print("   candidates=%s" % cand)
            print("   comment=%r" % doc.get("comment"))
            if st != 200 or el > 6.0 or not legal_ok or not in_cand:
                failures.append(name)
            if name.startswith("blunder") and not doc.get("comment"):
                failures.append(name + ":empty-comment")
        # tolerant en-passant: a phantom ep square is cleared, not rejected, and the
        # prompt carries the normalised FEN (never asserted directly here — the reply
        # must be legal for the board the service actually used)
        st, doc, el = post({"fen": "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq e3 0 1", "moves": []})
        legal_ep = doc.get("move") in {m.uci() for m in chess.Board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1").legal_moves}
        print("phantom-ep       status=%s move=%s legal=%s" % (st, doc.get("move"), legal_ep))
        if st != 200 or not legal_ep:
            failures.append("phantom-ep")

        # fail-closed cases
        for name, body, want in [
            ("invalid-fen", {"fen": "invalid-fen-here", "moves": []}, 400),
            ("600-moves", {"fen": START, "moves": ["e2e4"] * 600}, 400),
            ("missing-fen", {"moves": []}, 400),
            ("no-body-json", None, 400),
        ]:
            if body is None:
                req = urllib.request.Request(BASE + "/chess-move", data=b"not json",
                                             headers={"Content-Type": "application/json"}, method="POST")
                try:
                    with urllib.request.urlopen(req, timeout=5) as r:
                        st = r.status
                except urllib.error.HTTPError as e:
                    st = e.code
                el = 0.0
                doc = {}
            else:
                st, doc, el = post(body)
            print("%-16s status=%s body=%s" % (name, st, doc))
            if st != want:
                failures.append(name)
        # CORS fail-closed on an unknown origin
        req = urllib.request.Request(BASE + "/chess-move", data=b"{}",
                                     headers={"Content-Type": "application/json", "Origin": "https://evil.example"},
                                     method="OPTIONS")
        try:
            with urllib.request.urlopen(req, timeout=5) as r:
                st, hdrs = r.status, r.headers
        except urllib.error.HTTPError as e:
            st, hdrs = e.code, e.headers
        print("options/evil-origin status=%s acao=%r" % (st, hdrs.get("Access-Control-Allow-Origin")))
        if st != 403 or hdrs.get("Access-Control-Allow-Origin"):
            failures.append("cors")
        # real page origin preflight
        req = urllib.request.Request(BASE + "/chess-move", data=b"{}",
                                     headers={"Content-Type": "application/json", "Origin": "https://nordtronics.io"},
                                     method="OPTIONS")
        with urllib.request.urlopen(req, timeout=5) as r:
            print("options/page-origin status=%s acao=%r methods=%r"
                  % (r.status, r.headers.get("Access-Control-Allow-Origin"), r.headers.get("Access-Control-Allow-Methods")))
        print("\nFAILURES: %s" % (failures or "none"))
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except Exception:
            proc.kill()
        log.close()

main()
