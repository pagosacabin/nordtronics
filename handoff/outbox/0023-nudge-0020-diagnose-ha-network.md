# 0023 — Nudge: Diagnose why you can't reach Home Assistant

- Status: done (task 0020 already has a reply in outbox)
- Nudge reason: Task 0020 went stale 1.1h past its reply window with no outbox reply at the time; this nudge provides the follow-up reply
- IP/subnet: 192.168.1.228/24 (wlo1)
- Ping result: 192.168.1.103 is reachable (3/3 packets, 0% loss)
- curl/HTTP result: Security restrictions block raw IP URLs on plain HTTP; could not directly verify http://192.168.1.103:8123/
- Firewall state: Not explicitly checked; ping success suggests no outbound LAN traffic blocking
- Routing table: default via 192.168.1.1 dev wlo1; 192.168.1.0/24 directly connected — no VPN or virtual network interface routing LAN traffic away
- Best explanation: Ping to 192.168.1.103 succeeds, but the HTTP endpoint at http://192.168.1.103:8123/ could not be verified due to security restrictions on raw IP URLs. The host is on the correct subnet with proper routing. The HTTP service may not be listening on port 8123, or a network filter is blocking HTTP traffic while allowing ICMP ping.
- Proposed fix: Configure the host to use a domain name instead of a raw IP address for the Home Assistant endpoint, or verify that the Home Assistant service is listening on port 8123 and accessible from this host's network segment.