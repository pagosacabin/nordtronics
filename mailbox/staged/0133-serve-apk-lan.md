---
task_id: "0133"
protocol_version: 1.0.0
status: staged
expect-reply-within: 2h
iteration: 1
proof:
  - branch: none
    note: "host-only serve task -- no branch was created and nothing was pushed except
      the mailbox state transitions on main. The deliverable is a live HTTP listener
      bound to the laptop's LAN address, which cannot live in a git branch."
  - run: none
    note: "no CI run covers this deliverable and none can: no repository file changed,
      so no workflow could have been triggered by the task itself. Enumerated, not
      asserted: .github/workflows/ holds android-build.yml, android-companion-v0.yml,
      backend.yml, detection-sim.yml, platformio.yml, website-check.yml. The three
      build workflows fire on their own branch allow-lists and path filters and none
      of their paths changed; the only runs on main for this run's commits are
      'Website Check' on the mailbox-transition commits (pickup 02e2b64 = success,
      run 37858472415), which check the website, not this task."
  - files: []
    reason: "no repository file changed -- an HTTP server was started on the host and
      the APK served in place; the only commits this run made to main are the mailbox
      transitions (pickup 02e2b64, this staging)."
  - served_file:
      path: /home/astroboy/nordtronics-release/0130/nordtronics-companion-v0-0130-release-signed.apk
      bytes: 2834828
      sha256: 0313c12eac4f83aeb91b13d8eefec671222d0abc78e82a746c0f1716bbd98ee1
      sha256_check: "matches the '0313c12e...' the task specifies, and matches the
        digest recorded in the 0130 staged reply and in the sibling evidence file
        signing-and-perms.txt in the same directory. Checked before serving."
      moved_or_copied: false
  - url: http://192.168.1.228:8765/nordtronics-companion-v0-0130-release-signed.apk
    listener: "python3 -m http.server 8765 --bind 192.168.1.228 --directory /home/astroboy/nordtronics-release/0130/"
    bind_scope: "192.168.1.228 only (wlo1, the home-LAN interface) -- NOT 0.0.0.0, NOT
      a public interface. Docker bridge 172.17.0.1 and loopback are not served."
    head_check: "HTTP/1.0 200 OK; Content-type: application/vnd.android.package-archive;
      Content-Length: 2834828"
    get_check: "a full GET over the LAN address returned http_code=200 size=2834828
      and hashed to 0313c12eac4f83aeb91b13d8eefec671222d0abc78e82a746c0f1716bbd98ee1
      -- i.e. the bytes a phone downloads are the verified artifact, not just what the
      header claims."
    process: "detached via setsid (PPID 1, own session and process group), so it
      survives the end of this cron run and the agent session."
    fallback_url: "http://home.local:8765/nordtronics-companion-v0-0130-release-signed.apk
      (avahi-daemon active on this host; use if the DHCP address moves)"
  - evidence: "sha256sum of the APK in place; curl -sI (200 + content length); a full
      curl GET re-hashed to the same digest; ss -ltnp showing the bind address; ps
      showing the detached listener. Transcript lines are quoted in the reply body.
      The API key value appears nowhere in this reply, in the served directory listing,
      or in the server log -- the served directory's signing-and-perms.txt records only
      public digests and explicitly withholds the key value."
notes: |
  STAGED (iteration 1), 2026-10-08 23:15-23:20 UTC. Off-peak (PEAK: OFF-PEAK 23:15
  UTC). PROTOCOL: MATCH protocol_version=1.0.0 (origin/main:mailbox/README.md).
  Model tier: DeepSeek Flash (deepseek-flash / provider deepseek).

  WHAT CHANGED. Nothing in the repository. One host-side action: a detached
  `python3 -m http.server` on port 8765 bound to the laptop's LAN address
  192.168.1.228 (wlo1), serving /home/astroboy/nordtronics-release/0130/ in place.
  The APK was NOT moved, renamed or copied -- the constraint is honoured literally:
  the served path is the original path 0130 produced.

  DOWNLOAD URL REPORTED TO STEPHEN (open on the phone, same home Wi-Fi):
      http://192.168.1.228:8765/nordtronics-companion-v0-0130-release-signed.apk
  mDNS is active on this host (avahi-daemon), so if the DHCP lease changes the same
  server is usually also reachable as:
      http://home.local:8765/nordtronics-companion-v0-0130-release-signed.apk
  The numeric LAN URL is the one to use; the .local name is a fallback.

  VERIFICATION PERFORMED (read from the host, not assumed):
  - sha256sum of the file in place ->
    0313c12eac4f83aeb91b13d8eefec671222d0abc78e82a746c0f1716bbd98ee1 -- byte-for-byte
    the '0313c12e...' the task requires, checked BEFORE starting the server.
  - curl -sI http://192.168.1.228:8765/nordtronics-companion-v0-0130-release-signed.apk
      HTTP/1.0 200 OK
      Content-type: application/vnd.android.package-archive
      Content-Length: 2834828
  - a full GET through the same URL (not just a HEAD) returned http_code=200
    size=2834828 and re-hashed to 0313c12eac4f83aeb91b13d8eefec671222d0abc78e82a746c0f1716bbd98ee1.
    This is the check that the phone's download is actually the signed artifact.
  - ss -ltnp -> LISTEN 0 5 192.168.1.228:8765 0.0.0.0:* users:(("python3",pid=...)).
    Bound to the LAN address only; there is no 0.0.0.0 listener and no second bind.
  - ps -o pid,ppid,sid -> the server's PPID is 1 and it owns its own session, so the
    cron run ending does not take it with it.
  - firewall: the wlo1 zone (FedoraWorkstation) explicitly opens 1025-65535/tcp, and
    8765 is inside that range, so the port is reachable from the LAN without a new
    firewall rule. Confirmed with `firewall-cmd --list-all` rather than assumed; no
    sudo was needed and none was used.

  KEY SAFETY. The API key value appears nowhere in this reply. The server log is
  /home/astroboy/.cache/apk-serve/http-8765.log (request lines only). The served
  directory listing exposes three APK files plus signing-and-perms.txt; that file
  records only public digests (APK sha256, cert sha256) and states in its own text that
  the key value "never appears in any command line, argv or file in this directory".
  The directory listing itself was read to confirm no key is present in it. Serving
  the directory rather than a single file is exactly what the task specified; the two
  unsigned artifacts and the evidence file carry the same LAN-only exposure as the
  signed APK, which already contains the key by design.

  DEVIATION / RISK DECLARED. The LAN address is DHCP-assigned (wlo1 lease ~50 min and
  renewing), so the numeric URL can change if the lease is not renewed at the same
  address. If Stephen's download fails on that URL, re-read `hostname -I` on the laptop
  (or use http://home.local:8765/...) -- that is not a reason to re-run this task.

  STANDING DOWN as instructed: the server is left running until Stephen confirms the
  download, then it is stopped. No follow-up action is taken by this run.
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

---

# Reply — status: staged

## Download URL

    http://192.168.1.228:8765/nordtronics-companion-v0-0130-release-signed.apk

Fallback if the DHCP address moves (avahi/mDNS is active on this host):

    http://home.local:8765/nordtronics-companion-v0-0130-release-signed.apk

Open that link on the phone, on the same home Wi-Fi, and install. The APK is signed
with the production key (package com.nordtronics.companion, versionName 0.1) and
carries the baked X-API-Key, so the installed app talks to the live backend without
further setup.

## What was done

1. Verified the artifact before serving: `nordtronics-companion-v0-0130-release-signed.apk`
   is 2,834,828 B and sha256
   `0313c12eac4f83aeb91b13d8eefec671222d0abc78e82a746c0f1716bbd98ee1` — matching the
   `0313c12e...` named in this task and the digest recorded by 0130.
2. Started a detached HTTP server (own session, PPID 1, so it survives this run ending):

       python3 -m http.server 8765 --bind 192.168.1.228 \
         --directory /home/astroboy/nordtronics-release/0130/

   Bound to the LAN interface only, never 0.0.0.0. The APK was not moved, renamed or
   copied — it is served from its original path.
3. Confirmed the port is reachable from the LAN: the wlo1 firewalld zone
   (FedoraWorkstation) opens 1025-65535/tcp, which includes 8765.

## Proof (as specified by the task)

- sha256 of the served file:
  `0313c12eac4f83aeb91b13d8eefec671222d0abc78e82a746c0f1716bbd98ee1`
- `curl -sI` on the download URL:
  `HTTP/1.0 200 OK`, `Content-type: application/vnd.android.package-archive`,
  `Content-Length: 2834828`
- a full `curl` GET of that URL re-hashed to the same sha256, so the bytes a phone
  receives are the verified artifact.

No branch, no CI run and no changed repository file exist for this task — it is a
host-side serve. See the `proof` block above for the enumerated reasons.

## Notes

- The API key value appears nowhere in this reply, in the directory listing, or in the
  server log. Only public digests (APK, certificate) are exposed.
- The numeric URL is DHCP-derived; if it stops resolving, `http://home.local:8765/...`
  is the fallback, or re-read the address on the laptop with `hostname -I`.
- The server stays up until the download is confirmed, then it is stopped.
