---
task_id: "0133"
protocol_version: 1.0.0
status: in_progress
expect-reply-within: 2h
iteration: 1
---

# 0133 — Serve the 0130 release APK over the home LAN for Stephen's phone

## Context

- The signed release APK from 0130 is at /home/astroboy/nordtronics-release/0130/nordtronics-companion-v0-0130-release-signed.apk (2,834,828 B, sha256 0313c12e... per the 0130 staged reply).
- The backend auth gate is deployed and live. Stephen needs the APK on his Android phone to complete the end-to-end check (install, confirm live node data).
- Stephen is on his phone on the home LAN asking for a download link. There is none yet — make one.

## Task

1. Start a simple HTTP server on your laptop's LAN interface serving the directory /home/astroboy/nordtronics-release/0130/, e.g.:
   python3 -m http.server 8765 --bind <your-lan-ip> --directory /home/astroboy/nordtronics-release/0130/
   Run it in the background so it survives the task ending.
2. Verify it serves: curl -sI http://<your-lan-ip>:8765/nordtronics-companion-v0-0130-release-signed.apk should return 200 with the right content length.
3. Report the full download URL in your staged reply.

## Success criteria

- Stephen can open the reported URL on his phone (same home Wi-Fi) and download the APK.
- The served file's sha256 matches 0313c12e... (verify before serving).

## Constraints

- Bind to the LAN interface only — do not expose this beyond the home network. This APK contains the shared API key.
- Do not move, rename, or copy the APK anywhere else. Serve it in place.
- Leave the server running until Stephen confirms the download; then stop it.

## Proof

- The sha256 check output and the curl -I 200 line (URL host may be shown; the key itself must not appear anywhere).

## Reply format

Staged reply per mailbox/README.md: status, the download URL, proof pointers above, notes.
