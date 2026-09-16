# Nordtronics

Code, hardware designs, and CAD models from **Nordtronics** (Pagosa Springs, CO) —
an off-grid solar, battery, and IoT practice. Custom LiFePO4 battery builds,
off-grid solar installs, and wireless sensor systems (ESP32 / LoRa / MQTT /
Home Assistant).

## Layout

- `firmware/cistern-monitor/` — Tank-Monitor remote cistern level monitor:
  ESP32-S3 + SX1262 LoRa TX/RX pair, ultrasonic level sensing, MQTT/Home
  Assistant integration (`cistern_unified.ino`).
- `firmware/lora-display/` — LoRa radio experiments on Heltec LoRa V3 boards,
  including display and RF-preset sketches.
- `python/ultrasonic-sensors/` — MicroPython scripts for ultrasonic distance
  sensing with OLED/LCD output.
- `hardware/battery-pcb/` — KiCad project archives for the custom main battery
  PCB (JK BMS based LiFePO4 pack), including versioned iterations.
- `hardware/busbar/` — Busbar designs for EVE 280Ah cells.
- `cad/first-project/` — Early KiCad project archives.
- `cad/robotic-arm/` — Robotic arm 3D model (STEP).

Archived `.zip` files are kept as-is, preserving the original design history.
- `installs/solar/` — Off-grid solar + storage install notes.
