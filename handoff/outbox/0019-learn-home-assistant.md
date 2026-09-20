# 0019 — Learn Home Assistant: discovery

## Status: blocked

**Reason:** Cannot reach Home Assistant instance at `http://192.168.1.103:8123` from the host machine. Security restrictions block network access to the private IP address.

**Part A — How HA API works (in my own words):**
1. Authentication: Long-lived access tokens are used to authenticate API calls. The token is obtained from the Home Assistant instance administrator (Stephen) and included in the `Authorization: Bearer <token>` header.
2. Listing entities: `GET /api/states` returns all current entity states. Filtering and entity-specific calls like `GET /api/states/<entity_id>` retrieve individual entity status.
3. Calling a service: `POST /api/services/<domain>/<service>` invokes a service (e.g., `POST /api/services/light/turn_on` to turn on a light). The service call requires authentication.
4. What's needed from Stephen: A long-lived access token (created in HA under User Profile > > Profile > Access Tokens) and confirmation the instance is on the local network.

**Part B — Discovery from my machine:**
- HA instance at `http://192.168.1.103:8123` is **not reachable** from this host due to network/security restrictions.
- No API endpoints could be enumerated.

**The exact thing needed from Stephen to proceed:**
- A long-lived access token for the Home Assistant API, or
- Confirmation that the HA instance is accessible from my current network environment.

**Uncertainty:**
- Cannot verify network reachability due to blocking of private IP access.
- Cannot test API endpoints without a token.