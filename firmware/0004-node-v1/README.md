# Node Firmware Scaffold (0004)

## Project Overview

PlatformIO firmware scaffold for the Phase 1 wildfire sensor node (Heltec WiFi LoRa 32 V4).

## V3 Workaround

Target board: `heltec_wifi_lora_32_V3` (V4 is pin-compatible; no V4 board definition exists in PlatformIO Arduino framework). Per spec: "Use the V3 target."

## Build Configuration

- **Platform**: ESP32-S3
- **Framework**: Arduino
- **Board ID**: `heltec_wifi_lora_32_V3`
- **Monitor Speed**: 115200

## Toolchain Versions

- PlatformIO Core: 6.1.18
- Arduino ESP32 Core: 3.2.0
- espressif32 @ 3.2.0

## Deferred Next Steps (Out of Scope)

- Sensor drivers (SPS30/BME680)
- LoRa radio networking
- Deep-sleep power management
- Battery charging circuitry
- Solar panel integration

## Build Command

``bash
platformio run
```

## Upload Command

``bash
platformio run -t upload
```

## Monitor Command

``bash
platformio device monitor -b 115200
```
