---
task_id: "0072"
protocol_version: 1.0.0
status: inbox
iteration: 1
expect-reply-within: 6h
---

# 0072 — Encrypted off-site backup of the production keystore

## Context

Task 0071 minted the PRODUCTION release keystore on this machine:
- `~/.config/nordtronics/nordtronics-release.jks` (0600, PKCS12 despite the extension)
- `~/.config/nordtronics/release-keystore.pw` (0600, the only copy of the password)
- Alias: `nordtronics-release`. Cert SHA-256: `e4389ce4cb173da155b63cadf24c999723130c209bd55ca2e11bbf82a2a44ddd`

Stephen approved storing an encrypted backup bundle on his VPS
(`deploy@89.117.21.105`), encrypted with a passphrase only he holds.
Model: deepseek-flash. Report the model used in your reply.

## Task

One deliverable: a GPG-encrypted bundle of the two files above, verified by
test-decryption, uploaded to the VPS. The VPS must only ever hold the
encrypted blob — never plaintext secrets.

## Success criteria

- Ask Stephen for the backup passphrase through your normal local channel
  (you run on his laptop — talk to him directly, not through any file).
- `tar czf` the two files, then `gpg --symmetric --cipher-algo AES256` the
  tarball. Output name: `nordtronics-release-key-backup-2026-09-27.tar.gz.gpg`.
- Verify the bundle: decrypt to a temp directory, `sha256sum` both files
  against the originals, then securely wipe the temp directory (`shred -u`
  or `rm -P`; say which).
- `scp` the `.gpg` bundle to `deploy@89.117.21.105:/home/deploy/backups/`
  and `chmod 600` it there. Create `~/backups/` on the VPS if missing.
- If SSH to the VPS does not work out of the box from this machine, do NOT
  improvise credentials: leave the verified bundle at a stable local path and
  report that path instead.
- The passphrase must NEVER appear in the repo, the reply, any evidence
  file, or any log. Report paths and hashes only.

## Constraints

- Do NOT commit the bundle, the keystore, the password file, or the
  passphrase to the repo. Nothing about this task changes the repository.
- Do not alter the originals in `~/.config/nordtronics/` — they stay live
  for future signing work.
- Cost bound: deepseek-flash. Report the model in your reply.

## Proof

- `gpg --list-packets` (first lines, showing `symkey-enc` / AES256) — pasted.
- The sha256 of the originals vs the test-decrypted copies (must match).
- Either: `ls -l` of the bundle at `/home/deploy/backups/` on the VPS,
  or the exact local path where the verified bundle sits and why scp was
  not possible.

## Reply format

Stage the reply in `mailbox/staged/` per `mailbox/README.md`, with front
matter `status: staged`, the proof above, and a notes field stating where
the encrypted bundle lives, that the temp decrypted copies were wiped, and
a reminder that the passphrase is Stephen's to keep (password manager).
