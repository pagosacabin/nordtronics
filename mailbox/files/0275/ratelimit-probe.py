#!/usr/bin/env python3
"""Rate-limit probe: with burst=3 / 6 per min, the 4th rapid request must be 429."""
import json, os, subprocess, sys, time, urllib.request, urllib.error

APP = "/home/astroboy/wt/0275-pocket-conquest/pocket-conquest-bridge"
env = dict(os.environ)
env.update(PC_BRIDGE_RATE_PER_MIN="6", PC_BRIDGE_RATE_BURST="3",
           PC_BRIDGE_PORT="8770", DEEPSEEK_API_KEY="sk-probe-not-used")
proc = subprocess.Popen([sys.executable, APP + "/server.py"], env=env,
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
time.sleep(1.5)
try:
    codes = []
    for _ in range(5):
        req = urllib.request.Request("http://127.0.0.1:8770/pocket-conquest-move",
                                     data=b"{not json", method="POST")
        req.add_header("Content-Type", "application/json")
        try:
            codes.append(urllib.request.urlopen(req, timeout=5).status)
        except urllib.error.HTTPError as exc:
            codes.append(exc.code)
    print("burst=3, 6/min -> first five rapid POSTs:", codes)
    print("4th and 5th are 429:", codes[3] == 429 and codes[4] == 429)
finally:
    proc.terminate()
    try: proc.wait(timeout=5)
    except subprocess.TimeoutExpired: proc.kill()
