---
task_id: "0057"
protocol_version: 1.0.0
status: staged
iteration: 1
expect-reply-within: 6h
proof:
  - public_key: "ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIALtV/zLF6UeBHXe8EBSGBy+GOCDv72J4aQ68zvGKgYn nordlus@gmail.com"
  - public_key_path: /home/astroboy/.ssh/id_ed25519.pub
  - private_key_path: /home/astroboy/.ssh/id_ed25519  # mode 600, never read or transmitted
  - fingerprint: SHA256:6EtXtLADDPItdy6eLCVzxoIfOd6fzO7ef/g8lT2csBI
  - local_user: astroboy
  - deliverable: no code artifact for this task — the public key itself is the artifact ("The staged reply containing the public key is the proof")
notes: >
  No key generated — an ed25519 keypair already existed at ~/.ssh/id_ed25519
  (created 2025-12-08), so ssh-keygen was not run. Steps actually performed
  (cron run 2026-09-25, user astroboy, HOME=/home/astroboy): (1) read the .pub
  file only — the private key was never printed, copied, or staged, and its 600
  mode is unchanged; (2) `ssh-keygen -l -f ~/.ssh/id_ed25519.pub` for the
  fingerprint and type; (3) a pair check — the public blob derived from the
  private key (`ssh-keygen -y -P ""`) is identical to the blob in the .pub, and
  the private key has an EMPTY passphrase, so headless cron SSH will not stall
  on a passphrase prompt. Not tested and not claimed: connectivity to the VPS —
  no access exists yet, so there is nothing to verify there; the next VPS task
  proves it by SSHing in and reporting the hostname.
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

---

# Reply

| Field | Value |
|---|---|
| Local user | `astroboy` |
| Public key path | `/home/astroboy/.ssh/id_ed25519.pub` |
| Private key path (not shared) | `/home/astroboy/.ssh/id_ed25519` (mode `600`) |
| Key type | `ed25519` |
| Fingerprint | `SHA256:6EtXtLADDPItdy6eLCVzxoIfOd6fzO7ef/g8lT2csBI` |
| Comment | `nordlus@gmail.com` |
| Passphrase | empty — usable from non-interactive cron jobs |

Public key — one line, exactly as it appears in the `.pub` file:

```
ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAIALtV/zLF6UeBHXe8EBSGBy+GOCDv72J4aQ68zvGKgYn nordlus@gmail.com
```

The pair already existed (created 2025-12-08), so nothing was regenerated; this
is the same key I already use for GitHub over SSH, and installing it on the VPS
does not change anything about my existing access.

Which user/key this belongs to: the scheduled mailbox worker (this cron job)
runs as local user `astroboy` with `HOME=/home/astroboy`, under the `cronrunner`
Hermes profile. That is precisely the identity that will open the future SSH
sessions to `deploy@89.117.21.105`, so installing this key in `deploy`'s
`~/.ssh/authorized_keys` lets those runs connect unattended.

Next step is yours: install the key on the VPS. I will not attempt to connect
until you confirm it is in place — the first VPS task will then verify by
reporting the remote hostname.
