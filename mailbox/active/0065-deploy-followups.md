---
task_id: "0065"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0065 — Deploy follow-ups: mosquitto file ownership, API vhost hardening, runbook doc fixes (branch only)

## Context

0064 (verified, archived) finished the backend deploy. Its reply left
four small follow-ups open. Branch from the 0063 tip:
`hermes/0063-deploy-abc-fix @ 95de2ae6bdd7b60906a6583b941175b603694715`.
Name the new branch `hermes/0065-deploy-followups`.

## Task

On the new branch, make these `backend/DEPLOY.md` changes (no VPS
changes in this task):

1. Mosquitto file ownership: mosquitto warns that
   `/etc/mosquitto/passwd` and `/etc/mosquitto/acl` are not owned by the
   `mosquitto` user, and future versions will refuse to load them. Add a
   runbook step setting ownership to `mosquitto:mosquitto` (mode 0600 for
   passwd, 0640 for acl) and reloading mosquitto.
2. API vhost hardening: the appended api vhost in
   `/etc/nginx/sites-available/nordtronics.io` is missing the
   tls/headers snippets the main site uses. Add the include lines so the
   API vhost matches the site's TLS and header hardening.
3. Runbook step-10 doc fix: the plaintext-1883 check returns "Connection
   refused" when run on the VPS itself (a host reaching its own public IP
   goes via lo, which UFW accepts); the expected timeout output is only
   reachable from an off-host machine. Correct the expected-output note.
4. Operator bundle access (decision, already made): add
   `setfacl -m u:deploy:r /etc/nordtronics/mqtt-ca.pem` to the runbook's
   bundle step, following the 0061 setfacl pattern, so `deploy` can probe
   the bundle directly.

Push the branch and get CI green.

## Success criteria

1. CI is green on `hermes/0065-deploy-followups`.
2. The diff implements exactly the four items above.

## Constraints

- Branch only: zero VPS changes. A follow-up task applies this on the VPS.
- Cost bound: deepseek-flash only, off-peak hours.

## Proof

Branch SHA on origin + Actions run URL showing green, both quoted in the
reply.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: the branch SHA, the CI run URL and result, and the diff stat.
