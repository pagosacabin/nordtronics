# ESPHome device skeleton (v1)

One base config for every ESP32 board we build, so each project starts from the same
proven firmware instead of a blank page. It exists because the propane bench build
(0208–0212) lost a morning to a WiFi credential typed into a file, read by hand and
flashed as a secret — a class of mistake the portal removes.

## The three layers

1. **Portal provisioning** — `wifi: ap:` plus `captive_portal:`. The board raises its own
   setup page; the house network and passphrase are entered there, on the phone, and stored
   in NVS. **No station credentials are ever committed, staged or handed to a file again.**
   The AP password is a substitution, labelled in the YAML as guarding only the setup page.
2. **Firmware defaults** — identical on every board: native `api:` with an encrypted key,
   `ota:`, `logger:`, SNTP time, and the standard diagnostics (uptime, WiFi signal, IP
   address, restart). This is the part that should never drift between projects.
3. **Per-device settings** — a marked section where each project declares its own values.
   The worked example is a single persisted template `number`; the rule is that every
   threshold, offset or interval lives there, persisted on the board (`restore_value`) and
   adjustable from Home Assistant after adoption.

## Starting a new device

1. Copy `esphome-base.yaml` to `firmware/<project>/<project>.yaml`.
2. Set the substitutions: `device_name` (kebab-case, unique), `friendly_name`, and `board`
   if it is not an ESP32-C3 Super Mini (`esp32-s3-devkitc-1` for S3 boards).
3. Add the project's own code **below** the skeleton — never inside layers 1 and 2.
4. Create a local `secrets.yaml` next to the config (mode 600, never committed, listed in
   `.gitignore`) with a freshly generated key:
   `api_encryption_key:` + the output of
   `python3 -c "import base64,os;print(base64.b64encode(os.urandom(32)).decode())"`.
   A key must be exactly 32 bytes base64 (44 characters); the value in
   `secrets.yaml.example` is a valid-format placeholder, not a usable key.
   Only `secrets.yaml.example` is committed.
5. `esphome config <project>.yaml` then `esphome run <project>.yaml` to flash.
6. On the phone: join the board's `<device_name> setup` AP, open the setup page, pick the
   house network and enter its passphrase. That is the only place the house credential is
   typed.
7. Adopt in Home Assistant (it asks for the API encryption key once), then set the
   per-device values from the device page.

## Rules

- **House WiFi credentials are never committed or staged in a file again.** The portal is
  the provisioning path; the fallback AP password is not a secret and is labelled as such.
- `secrets.yaml` is local and uncommitted. The repo carries only
  `secrets.yaml.example` with placeholders.
- Infrastructure addresses are firmware defaults, never portal fields or per-device
  settings.
- Keep v1 minimal: no project sensors, no MQTT, no web server. Extend the base only once a
  real board has proven the change.

## Verified base

`esphome config` and `esphome compile` were run against this skeleton with a generated
`secrets.yaml`; see the task reply for the ESPHome version and the compile result.
