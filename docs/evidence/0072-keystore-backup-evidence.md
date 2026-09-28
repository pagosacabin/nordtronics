# 0072 evidence — encrypted off-site backup of the production keystore

Task: `0072` — GPG-encrypted bundle of the 0071 production keystore uploaded to the VPS.
Model used: `deepseek-flash` (provider `deepseek`).

**No secret appears in this file.** No bundle, keystore, password file, or passphrase is
committed. The tarball password and key material live only on this host and on the VPS as
ciphertext.

## Deliverable

| | |
|---|---|
| Bundle | `nordtronics-release-key-backup-2026-09-27.tar.gz.gpg` |
| Local path | `/home/astroboy/backups/nordtronics-release-key-backup-2026-09-27.tar.gz.gpg` |
| Local mode/size | `-rw------- ` 0600, 4640 bytes |
| VPS path | `/home/deploy/backups/nordtronics-release-key-backup-2026-09-27.tar.gz.gpg` |
| VPS mode/size | `-rw-------` 0600 (`deploy:deploy`), 4640 bytes |
| Bundle sha256 | `8b65971419cde5c3ddd83a3b0441d732c4a3af4a12084046d42deef4a80ab6b7` |

Both copies hash identically, so the uploaded blob is byte-for-byte the locally
test-decrypted-and-verified bundle.

## 1. Archive contents (the only two inputs)

```
tar tzvf bundle.tar.gz
-rw------- astroboy/astroboy 4386 2026-09-27 00:16 nordtronics-release.jks
-rw------- astroboy/astroboy   44 2026-09-27 00:16 release-keystore.pw
```

## 2. Cipher — `gpg --list-packets` head

```
gpg: AES256.CFB encrypted data
gpg: encrypted with 1 passphrase
# off=0 ctb=8c tag=3 hlen=2 plen=13
:symkey enc packet: version 4, cipher 9, aead 0, s2k 3, hash 10
	salt 123F01B438C82475, count 65011712 (255)
# off=15 ctb=d2 tag=18 hlen=3 plen=4622 new-ctb
:encrypted data packet:
	length: 4622
	mdc_method: 2
# off=37 ctb=ad tag=11 hlen=3 plen=4578
:literal data packet:
	mode b (62), created 1790601599, name="bundle.tar.gz",
	raw data: 4559 bytes
```

`cipher 9` = AES256, `s2k 3` = iterated+salted, `hash 10` = SHA512, `mdc_method: 2`
= integrity-protected. Command was
`gpg --batch --yes --pinentry-mode loopback --passphrase-file <file> --symmetric --cipher-algo AES256`.

## 3. Round-trip verification (originals vs test-decrypted copies)

```
--- originals ---
ed6da2fef7b9dd638f2af9621406b5640f65317258340225b2677ecad97f9537  nordtronics-release.jks
d92853e256866d3d8f0b907816b8418ae82a727e72ed0c0973ac3cde4c232ef8  release-keystore.pw
--- test-decrypted copies ---
ed6da2fef7b9dd638f2af9621406b5640f65317258340225b2677ecad97f9537  nordtronics-release.jks
d92853e256866d3d8f0b907816b8418ae82a727e72ed0c0973ac3cde4c232ef8  release-keystore.pw
HASH MATCH: both files identical after round-trip
```

The bundle was decrypted into a scratch dir, extracted, and compared against the originals.
Both pairs match, so the archive is decryptable with the held passphrase and contains
exactly the source bytes.

## 4. Plaintext wipe

Wiped with `shred -u` (not `rm -P`): the two decrypted copies, the decrypted
`decrypted.tar.gz`, and the plaintext `bundle.tar.gz`. The scratch dir
`$HOME/.hermes/cache/scratch/0072-work` was then removed (`rmdir`), confirmed absent
afterwards. No plaintext copy of either input file exists anywhere on disk.

## 5. VPS state

```
$ ssh deploy@89.117.21.105 'chmod 700 /home/deploy/backups; chmod 600 *.gpg; ls -l; sha256sum *.gpg'
created: drwx------ 2 deploy deploy 4096 Sep 28 15:20 /home/deploy/backups
--- VPS side ---
total 8
-rw------- 1 deploy deploy 4640 Sep 28 15:20 nordtronics-release-key-backup-2026-09-27.tar.gz.gpg
8b65971419cde5c3ddd83a3b0441d732c4a3af4a12084046d42deef4a80ab6b7  nordtronics-release-key-backup-2026-09-27.tar.gz.gpg
```

`/home/deploy/backups/` was created by this task (it did not exist), mode 700; the blob is
0600. The VPS holds only the ciphertext. SSH used the existing 0057 key unattended
(`BatchMode=yes`); no credential was improvised.

## 6. No CI

Nothing in the repository changed except this evidence file, so this task has **no Actions
run** and the reply deliberately carries no `run:` pointer.
