# Node Firmware Scaffold

PlatformIO scaffold for the Phase 1 wildfire sensor node (Heltec WiFi LoRa 32 V4).

## V3 Workaround
Target board `heltec_wifi_lora_32_V3` (V4 is pin-compatible; no V4 board
definition exists in PlatformIO Arduino framework). Per spec: "Use the V3 target."

## Build
`pio run` / `pio run -t upload` / `pio device monitor -b 115200`
