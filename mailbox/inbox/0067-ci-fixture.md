---
task_id: "0067"
protocol_version: 1.0.0
status: inbox
iteration: 0
expect-reply-within: 6h
---

# 0067 — CI fixture: guard the mosquitto passwd/acl ownership rule (branch only)

## Context

0065's notes declared one gap: CI does not exercise the new mosquitto
passwd/acl ownership rule — `backend/mosquitto/test-fixture.sh` still
creates the fixtures as `root:mosquitto` 0640, so the CI broker probe
builds them the old way and the warning is never asserted. The CI broker
is mosquitto 2.0.18, the same version that emits the warning on the VPS.
Branch from the 0065 tip: `hermes/0065-deploy-followups @
a4b1724772c8e4ca576a1f2b185a5b6c55dcc24f`. Name the new branch
`hermes/0067-ci-mosquitto-ownership`.

## Task

On the new branch, change `backend/mosquitto/test-fixture.sh` so the
passwd/acl fixtures are created as `mosquitto:mosquitto`, mode 0600 for
passwd and 0640 for acl — matching the runbook rule 0065 added. If the
CI environment cannot chown to the mosquitto user, say so in the reply
instead of faking it; do not weaken the assertion to pass.

Push the branch and get CI green.

## Success criteria

1. CI is green on `hermes/0067-ci-mosquitto-ownership`.
2. The fixture creates passwd/acl with the runbook's ownership and
   modes, so CI guards the rule.

## Constraints

- Branch only: zero VPS changes.
- Cost bound: deepseek-flash only, off-peak hours.

## Proof

Branch SHA on origin + Actions run URL showing green, both quoted in the
reply.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body: the branch SHA, the CI run URL and result, and the diff stat.
