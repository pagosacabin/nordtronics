# Tank Monitor Firmware

PlatformIO project for Nordtronics cistern monitoring system.

## Features

- Ultrasonic tank level measurement
- LoRa wireless transmission
- MQTT integration
- Automatic mode detection (transmitter/receiver)
- Deep sleep power saving
- WiFi connectivity

## Hardware

- Heltec LoRa 32 (ESP32)
- Ultrasonic sensor (HC-SR04)
- LoRa radio module

## Getting Started

1. Connect to WiFi
2. Configure MQTT broker
3. Upload firmware via PlatformIO
4. Monitor via Serial Monitor or OLED display

## Notes

- Mode is auto-detected based on sensor presence
- Manual override via PRG button during boot
- Sensors on GPIO 13/12 detect transmitter mode
