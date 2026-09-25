---
task_id: "0058"
protocol_version: 1.0.0
status: in_progress
iteration: 1
expect-reply-within: 6h
---

# 0058 — Test SSH access to the VPS

## Context

Your public key (0057, `/home/astroboy/.ssh/id_ed25519.pub`) is now installed
in `deploy@89.117.21.105:~/.ssh/authorized_keys`. Before anything deploys,
prove the connection works from your machine with your key.

## Task

SSH to the VPS as the `deploy` user and prove it:

```
ssh -o ConnectTimeout=20 deploy@89.117.21.105 'hostname && whoami && uptime'
```

Report the full output in your reply.

## Success criteria

1. The SSH connection succeeds using your key — no password prompt.
2. The reply quotes the command's output showing the VPS hostname.

## Constraints

- Read-only: run the one command only. Do not install, modify, or restart
  anything on the VPS in this task.
- If the connection fails, report the exact error output. Do not retry more
  than twice; do not work around it.
- Small task: deepseek-flash.

## Proof

The staged reply quoting the SSH command output is the proof.

## Reply format

Front-matter (task_id, protocol_version, status, iteration, proof), then the
reply body with the command and its output in a code block — or the exact
failure output if it did not connect.
