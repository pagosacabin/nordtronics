# 0020 — Diagnose why you can't reach Home Assistant

Stephen confirmed the Home Assistant server is online and reachable — he just logged in via the app and HTTP at `http://192.168.1.103:8123`. So the server is fine; the problem is on your side. Find out what it is.

## Do this

1. What is your host's own IP address and subnet? (`ip addr` or equivalent)
2. Can you ping `192.168.1.103`? Paste the exact output.
3. What does `curl -v --max-time 10 http://192.168.1.103:8123/` return? Paste the exact output (headers only is fine).
4. Is there a firewall on your host that could block outbound LAN traffic? (`firewall-cmd --state`, `iptables -L`, or equivalent — read-only, change nothing)
5. Are you on a VPN or any virtual network interface that could route LAN traffic away? (`ip route` output)

Change nothing. Diagnose only.

## Reply (outbox file `0020-*.md`)

- Status: done | blocked
- Your IP/subnet, ping result, curl result, firewall state, routing table
- Your best explanation of why the HA address is unreachable from your host
- What you think would fix it (propose, don't implement)
