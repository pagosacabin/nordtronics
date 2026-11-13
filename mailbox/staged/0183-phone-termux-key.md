---
task_id: "0183"
protocol_version: 1.2.0
status: staged
iteration: 1
expect-reply-within: 4h
cost:
  model: deepseek-flash
  api_calls: unknown
  input_tokens: unknown
  input_cache_hit_tokens: unknown
  input_cache_miss_tokens: unknown
  output_tokens: unknown
  total_tokens: unknown
  search_requests: 0
  rate_window: off-peak
  rates_used: "DeepSeek off-peak: $0.15/M input miss, $0.003/M input hit, $0.60/M output"
  cost_usd: unknown
  reason: >-
    Shared interactive session. The harness aggregates usage per session
    (state.db session_model_usage), not per task, and this session carried
    unrelated work before this task was picked up, so no per-call usage
    objects exist for 0183. Every figure is reported unknown rather than
    estimated from the session total.
proof:
  branch: hermes/0183-phone-key
  sha: e732f9b95a205e972fce7e2ac3f46ff1749116ca
  base: 96c6da3
  files: keys/phone-termux.pub
  file_sha256: 59d6a7983d7ce3a610e0175d108c268c679534f738e2e66222c9c89886f49440
  raw_url: https://raw.githubusercontent.com/pagosacabin/nordtronics/hermes/0183-phone-key/keys/phone-termux.pub
  run: none
  run_reason: >-
    No CI run applies: the deliverable is a public-key file, not a built
    artifact. The branch tip above is the whole proof pointer, and the file was
    fetched back over the raw URL to confirm what Stephen will download.
notes: "SHA256:YpcCodWz0b5Q8tyE4dpBzpFm2jo8/pP2QXKuw3MpiGc — dedicated ed25519 key for stephens-a35 Termux (comment hermes-to-stephens-a35-termux). Private key stays at ~/.ssh/phone-termux, mode 600, in no commit, reply or chat. VPS operator key id_ed25519 untouched (SHA256:6EtXtLADDPItdy6eLCVzxoIfOd6fzO7ef/g8lT2csBI re-checked after generation). No connection to the phone was attempted. Interactive-session hold: Fri mailbox worker 7aff6948c2c1 paused 12:10 MST, resumed after this staging commit."
---

# 0183 — Generate a dedicated SSH key for Stephen's phone (Termux) and stage the public key

## Assignment
- Hermes: all steps below, on the laptop. Newton: no steps.
- Stephen (outside this task, coordinated by Juno): after Juno relays the staged reply, he runs one curl line on the phone to install the public key into Termux `authorized_keys`. The test-connect task is filed separately once he confirms that step — same 0057 → 0058 split as the VPS key.

## Context
- Goal: Hermes gets a shell on Stephen's phone (Samsung A35, tailnet name `stephens-a35`, tailnet IP **100.103.159.65**) so future phone-side network faults can be diagnosed from the phone itself. Motivation: task 0181 (Nov 13) — HA unreachable was a dead phone Tailscale tunnel, diagnosable from the laptop only by inference; a phone shell closes that gap.
- Phone side is DONE by Stephen (Nov 13, screenshots to Juno): Termux (current build, live termux.net repo) with OpenSSH 10.3p1 installed; `sshd` started, listening on port **8022**; Termux user is **u0_a342**. Nothing on the phone accepts a login yet — no password set, `~/.ssh/authorized_keys` empty. This task supplies the key.
- Precedent: 0057 (Hermes generated/staged his ed25519 key) → Juno installed it on the VPS → 0058 proved the connection. Same shape here, with one change: a **dedicated** keypair for the phone, NOT the existing VPS operator key at `/home/astroboy/.ssh/id_ed25519` (fingerprint SHA256:6EtXtLADDPItdy6eLCVzxoIfOd6fzO7ef/g8lT2csBI). Phone access must be revocable without touching VPS access.
- The nordtronics repo is public (Stephen approved, 2026-09-15). A public key is not a secret, so publishing the .pub file in the repo is an approved delivery channel; the private key never leaves the laptop.

## Task
One deliverable: a dedicated keypair on the laptop plus the public key published and staged for relay to Stephen.

1. Generate the keypair (stop and report instead if either file already exists — include no overwrite):
   `ssh-keygen -t ed25519 -f ~/.ssh/phone-termux -N '' -C 'hermes-to-stephens-a35-termux'`
   Then `chmod 600 ~/.ssh/phone-termux`. Empty passphrase, matching the 0057 headless-use precedent.
2. Record the fingerprint: `ssh-keygen -lf ~/.ssh/phone-termux.pub`.
3. Publish the public key so Stephen can fetch it with one line on the phone: commit the exact contents of `~/.ssh/phone-termux.pub` as the repo file `keys/phone-termux.pub` on branch `hermes/0183-phone-key` in pagosacabin/nordtronics, using your proven git path. The file content is the single .pub line, nothing else.
4. Stage the reply (format below) carrying: the fingerprint, the full public-key line, the branch tip SHA, and the raw fetch URL:
   `https://raw.githubusercontent.com/pagosacabin/nordtronics/hermes/0183-phone-key/keys/phone-termux.pub`

## Success criteria
- `~/.ssh/phone-termux` and `~/.ssh/phone-termux.pub` exist on the laptop; the staged fingerprint matches `ssh-keygen -lf` output for that .pub.
- Branch `hermes/0183-phone-key` exists on origin, its tip equals the SHA in `proof`, and `keys/phone-termux.pub` on that branch is byte-identical to the local .pub file (one line).
- The staged reply contains the fingerprint, the full public-key line, and the raw URL from step 4.
- No connection to the phone is attempted in this task, and the existing VPS key (`id_ed25519`) is untouched.

## Constraints
- Include only the dedicated key above; use no existing key for this, and reuse this key for nothing else.
- The private key (`~/.ssh/phone-termux`) stays on the laptop: include it in no commit, no staged reply, no chat, no file outside `~/.ssh/`.
- Publish only the public key, only at the repo path and branch named above.
- Cost block in the staged front-matter per protocol 1.2.0.

## Proof
- `proof:` carries the branch + tip SHA of the `keys/phone-termux.pub` commit (Juno verifies the branch tip and fetches the file to compare against the staged key line), plus the pasted `ssh-keygen -lf` output in the body.

## Reply format
Stage the task file in `mailbox/staged/` with `status: staged`, the protocol cost block, `proof` as above, and a `notes` field whose first line is the key fingerprint. Body, in order: fingerprint line, full public-key line, raw URL, then the exact phone command Juno will hand Stephen:
`mkdir -p ~/.ssh && curl -sL <raw-url> >> ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys`
(with `<raw-url>` replaced by the real URL).

## Reply

Fingerprint (verbatim `ssh-keygen -lf ~/.ssh/phone-termux.pub`):

```
256 SHA256:YpcCodWz0b5Q8tyE4dpBzpFm2jo8/pP2QXKuw3MpiGc hermes-to-stephens-a35-termux (ED25519)
```

Public key, the single line published at `keys/phone-termux.pub`:

```
ssh-ed25519 AAAAC3NzaC1lZDI1NTE5AAAAICuMXwZBGQMoAoPwny3ASJW3cysEiGyJDTPra7EjCMBu hermes-to-stephens-a35-termux
```

Raw fetch URL:

```
https://raw.githubusercontent.com/pagosacabin/nordtronics/hermes/0183-phone-key/keys/phone-termux.pub
```

Exact command for Stephen to run in Termux:

```sh
mkdir -p ~/.ssh && curl -sL https://raw.githubusercontent.com/pagosacabin/nordtronics/hermes/0183-phone-key/keys/phone-termux.pub >> ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys
```

## Verification

Keypair created, nothing overwritten (both paths were absent beforehand):

```
-rw------- 1 astroboy astroboy 419 Nov 13 12:52 /home/astroboy/.ssh/phone-termux
-rw-r--r-- 1 astroboy astroboy 111 Nov 13 12:52 /home/astroboy/.ssh/phone-termux.pub
```

Branch tip on origin equals the SHA in `proof`:

```
$ git ls-remote --heads origin hermes/0183-phone-key
e732f9b95a205e972fce7e2ac3f46ff1749116ca	refs/heads/hermes/0183-phone-key
```

Published file is the .pub verbatim — one line, same bytes:

```
$ wc -l keys/phone-termux.pub      -> 1
$ sha256sum ~/.ssh/phone-termux.pub keys/phone-termux.pub
59d6a7983d7ce3a610e0175d108c268c679534f738e2e66222c9c89886f49440  /home/astroboy/.ssh/phone-termux.pub
59d6a7983d7ce3a610e0175d108c268c679534f738e2e66222c9c89886f49440  /home/astroboy/nordtronics/keys/phone-termux.pub
```

Fetched back over the raw URL Stephen will use, and compared byte-for-byte:

```
$ curl -fsSL https://raw.githubusercontent.com/pagosacabin/nordtronics/hermes/0183-phone-key/keys/phone-termux.pub -o raw.pub && sha256sum raw.pub
59d6a7983d7ce3a610e0175d108c268c679534f738e2e66222c9c89886f49440  raw.pub        # cmp against ~/.ssh/phone-termux.pub: identical
```

Constraints checked:

- Dedicated key only. The VPS operator key is unchanged: `ssh-keygen -lf ~/.ssh/id_ed25519.pub`
  still reads `256 SHA256:6EtXtLADDPItdy6eLCVzxoIfOd6fzO7ef/g8lT2csBI nordlus@gmail.com (ED25519)`.
- Private key never left `~/.ssh/`: not committed, not in this reply, not in chat.
- No connection to the phone was attempted in this task; the connect test stays a separate task
  after Stephen installs the key.

## Notes / deviations

- The keypair is new, so the `iteration` stayed at 1 across pickup and staging.
- Cost is `unknown` by the protocol's own escape hatch, not by omission — see the `reason`
  field; the session-level aggregate is deliberately not presented as this task's usage.
- The worker hold existed only to keep the 12:15/13:15 ticks off an `active/` task; staging
  landed well before the next fire.
