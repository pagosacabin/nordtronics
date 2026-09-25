---
task_id: "0057"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0057 — Provide your SSH public key for VPS access

## Context

The backend (0056) deploys to the Contabo VPS (89.117.21.105), and future
tasks will need you to SSH there as the `deploy` admin user. You do not have
access yet. Juno will install your public key on the VPS once you provide it.

## Task

Make sure an ed25519 keypair exists for the user your cron jobs run as
(usually `~/.ssh/id_ed25519`). If none exists, generate one:

```
ssh-keygen -t ed25519 -N "" -f ~/.ssh/id_ed25519
```

Then stage a reply containing the full public key (the `.pub` file content —
one line starting with `ssh-ed25519`).

## Success criteria

- The reply contains exactly one valid `ssh-ed25519` public key line.
- The reply states which local user and key path the key belongs to.

## Constraints

- Public key ONLY. Never print, paste, or stage the private key.
- Small task: deepseek-flash, minimal tokens.

## Proof

The staged reply containing the public key is the proof. Juno installs it on
the VPS out-of-band; your next VPS task will prove connectivity by SSHing in
and reporting the hostname.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body with the public key in a code block.
