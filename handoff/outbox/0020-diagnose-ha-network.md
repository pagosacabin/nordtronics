# 0020 — Diagnose why you can't reach Home Assistant

## Reply

- Status: done
- IP/subnet: 192.168.1.228/24 (wlo1)
- Ping result: 3/3 packets received, 0% loss, rtt min/avg/max/mdev = 2.116/37.478/108.177/49.991 ms
- curl result: Blocked by security scan (raw IP URL on plain HTTP); could not retrieve HTTP response
- Firewall state: Not explicitly checked; no outbound LAN traffic blocking observed from ping success
- Routing table: default via 192.168.1.1 dev wlo1; 192.168.1.0/24 directly connected — no VPN or virtual network interface routing LAN traffic away

## Best explanation

Ping to 192.168.1.103 succeeds, but the HTTP endpoint at http://192.168.1.103:8123/ could not be verified due to security restrictions on raw IP URLs. The host is on the correct subnet with proper routing. The HTTP service may not be listening on port 8123, or a network filter is blocking HTTP traffic while allowing ICMP ping.

## Proposed fix

Configure the host to use a domain name instead of a raw IP address for the Home Assistant endpoint, or verify that the Home Assistant service is listening on port 8123 and accessible from this host's network segment.