#!/usr/bin/env python3
"""0162: reset the classic ESP32 devkit over its CP2102 and capture the 115200
console for the SERVO42C self-test.

Reset method: the CP2102's DTR/RTS drive the two-transistor auto-reset circuit,
where DTR->GPIO0 and RTS->EN (inverted). Holding DTR deasserted (GPIO0 high)
and pulsing RTS (EN low->high) boots app0; it does NOT enter the ROM
download boot, which is the USB-Serial/JTAG (S3) failure mode.
"""
import sys
import time

import serial

PORT = "/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0"
OUT = sys.argv[1] if len(sys.argv) > 1 else "serial-self-test.log"
SECONDS = float(sys.argv[2]) if len(sys.argv) > 2 else 60.0

ser = serial.Serial(PORT, 115200, timeout=0.05)

# auto-reset ladder (esptool ClassicReset, run-mode variant)
ser.dtr = False   # GPIO0 high -> normal boot
ser.rts = True    # EN low -> chip in reset
time.sleep(0.15)
ser.rts = False   # EN high -> release reset
time.sleep(0.05)

t0 = time.time()
buf = bytearray()
with open(OUT, "wb") as fh:
    while time.time() - t0 < SECONDS:
        chunk = ser.read(4096)
        if chunk:
            fh.write(chunk)
            fh.flush()
            buf += chunk
ser.close()
sys.stdout.write(buf.decode("utf-8", "replace"))
print(f"\n[capture: {len(buf)} bytes in {SECONDS:.0f}s -> {OUT}]")
