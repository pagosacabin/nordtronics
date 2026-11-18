# Build and provision a board — the verified path (addendum to README.md)

Added 2026-11-18 by Hermes under task 0216. This is an **addendum**, not a rewrite of
`README.md`: the three-layer design there stands. What this file adds is the build and
provisioning path actually exercised on hardware, and the history that made it the rule.

## The path

1. **Build locally with `esphome`.** Not the Home Assistant add-on, not PlatformIO. The
   bench builds that work and fail here have both been local `esphome` 2026.9.1 builds,
   esp-idf framework:

   ```bash
   cd firmware/<project>
   printf 'api_encryption_key: "%s"\n' "$(python3 -c 'import base64,os;print(base64.b64encode(os.urandom(32)).decode())')" > secrets.yaml
   chmod 600 secrets.yaml          # never committed; only secrets.yaml.example is
   esphome config  <project>.yaml  # expect: INFO Configuration is valid!
   esphome compile <project>.yaml  # expect: INFO Successfully compiled program.
   esphome run     <project>.yaml  # flash over USB; expect: Hash of data verified.
   ```

   Keep a copy of that per-device `api_encryption_key`: it is the identity Home Assistant
   adopts, and regenerating it makes the board a new device.

2. **Provision WiFi at the board's own setup page.** The firmware carries `wifi: ap:` plus
   `captive_portal:` and **no station credentials**. On first boot join the
   `<device_name> setup` AP from the phone, pick the house network, type the passphrase
   there. It lands in NVS on the board. That is the only place a house credential is ever
   typed.

3. **Adopt in Home Assistant**, enter the API encryption key once, then set the per-device
   values from the device page. They persist on the board (`restore_value: true`).

## Status of each half

| Half | Evidence |
|---|---|
| Local `esphome` build + flash | **Verified on hardware** — propane bench board, `esphome` 2026.9.1 / esp-idf, `Hash of data verified.` on every upload; skeleton itself compiled for `esp32-c3-devkitm-1` (2026-11-18) |
| Portal provisioning | **Design (as of 2026-11-18)** — the first real board provisioned through the portal will be its proof. Nothing in this repo has been flashed with portal-only WiFi yet, so treat the phone walk-through as the intended path, not a measured one |

## Why this rule exists — corrected history

The propane bench build (0208) was handed a WiFi passphrase **staged in a file**, read by
Hermes and flashed as a secret. That credential was then compared three ways (file vs
flashed vs host PSK — all matching) and the join was still refused, so the file route cost
a morning and produced **no diagnosis at all**; the fault turned out to be upstream of the
credential (see below). Staged credential files are the mistake this skeleton removes.

**Correction, 2026-11-18 (task 0216):** the presence-sensor C3 — the build that *works* —
was **not** built through the Home Assistant add-on. It was a local `esphome` 2026.9.1
build using that project's own secrets file (`~/Documents/esphome/JC4827W543/secrets.yaml`
on Stephen's laptop), with a PSK byte-identical to the laptop's stored PSK for the same
network. An earlier account of it as an add-on build was Hermes's error, corrected
in-session by Stephen. Rebuilding the propane board exactly that way changed nothing: the
join still fails, which is what finally ruled the credential out for good.

## Where the propane board stands (so this file is not read as a claim of success)

The propane board is refused with `Auth Expired` on every attempt; the credential, build
method, framework, signal and MAC-collision explanations are all ruled out (task 0216).
Two suspects remain — the router refusing that client, or a faulty radio/efuse on that
Super Mini — and the separator test is a **2.4 GHz phone hotspot**: joins it, the board is
fine and the router is at fault; refused there too, the board is condemned and the rig
moves to a spare C3. Recorded in the crew vault, `bench/wifi-lessons.md`.
