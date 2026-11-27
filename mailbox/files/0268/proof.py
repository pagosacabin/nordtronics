import json, time, urllib.request, chess, subprocess, sys
BASE="http://127.0.0.1:8765"
cases=[
 ("start position","rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",[]),
 ("italian midgame","r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 4",["e2e4","e7e5","f1c4","b8c6","g1f3"]),
 ("kingside attack","r1bq1rk1/ppp2ppp/2np1n2/2b1p3/2B1P3/2NP1N2/PPPB1PPP/R2Q1RK1 w - - 0 8",[]),
]
print("== GET /health"); print(urllib.request.urlopen(BASE+"/health",timeout=5).read().decode())
print("\n== OPTIONS preflight (Origin https://nordtronics.io)")
req=urllib.request.Request(BASE+"/chess-move",method="OPTIONS",headers={"Origin":"https://nordtronics.io","Access-Control-Request-Method":"POST","Access-Control-Request-Headers":"content-type"})
r=urllib.request.urlopen(req,timeout=5); print("status",r.status,{k:v for k,v in r.headers.items() if k.lower().startswith("access-control")})
for name,fen,moves in cases:
    body=json.dumps({"fen":fen,"moves":moves}).encode()
    req=urllib.request.Request(BASE+"/chess-move",data=body,headers={"Content-Type":"application/json","Origin":"https://nordtronics.io"},method="POST")
    t=time.time(); resp=json.load(urllib.request.urlopen(req,timeout=6)); el=time.time()-t
    b=chess.Board(fen); ok=resp["move"] in {m.uci() for m in b.legal_moves}
    print("\n== POST %s\n   fen: %s\n   -> %s in %.2fs  legal=%s" % (name, fen, json.dumps(resp), el, ok))
    assert ok and el < 6.0
print("\n== fail-closed cases")
for label,payload in [("invalid fen",{"fen":"not-a-fen"}),("missing fen",{}) ,("oversized moves",{"fen":cases[0][1],"moves":["e2e4"]*600})]:
    req=urllib.request.Request(BASE+"/chess-move",data=json.dumps(payload).encode(),headers={"Content-Type":"application/json"},method="POST")
    try:
        urllib.request.urlopen(req,timeout=6)
        print("   %s -> 200 (UNEXPECTED)"%label)
    except urllib.error.HTTPError as e:
        print("   %s -> %d %s" % (label, e.code, e.read().decode()[:80]))
print("\n== journal (last 6)")
print(subprocess.run(["journalctl","--user","-u","chess-bridge.service","-n","6","--no-pager"],capture_output=True,text=True).stdout)
