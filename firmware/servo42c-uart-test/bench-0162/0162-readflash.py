#!/usr/bin/env python3
"""0162 helper: chunked, retried esptool read_flash + size assertion.

The 0161 run proved this uplink's flash read path is lossy on long transfers
("Corrupt data, expected 0x1000 bytes but received 0x10 bytes"), so every byte
that backs a hash here is read in 64 KB chunks with retries and a size check.
"""
import hashlib
import os
import subprocess
import sys
import time

PORT = "/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0"
ESPTOOL = "esptool.py"
CHUNK = 0x10000


def read_chunk(addr, size, out, attempts=5):
    for i in range(1, attempts + 1):
        if os.path.exists(out):
            os.unlink(out)
        cmd = [ESPTOOL, "--port", PORT, "--before", "no_reset", "--after", "no_reset",
               "read_flash", hex(addr), hex(size), out]
        r = subprocess.run(cmd, capture_output=True, text=True)
        ok_size = os.path.exists(out) and os.path.getsize(out) == size
        if r.returncode == 0 and ok_size:
            return True
        print(f"    retry {i}/{attempts} @ {addr:#x}: rc={r.returncode} "
              f"size={os.path.getsize(out) if os.path.exists(out) else 'missing'}",
              file=sys.stderr)
        time.sleep(1)
    return False


def read_region(addr, size, out):
    data = b""
    off = 0
    while off < size:
        n = min(CHUNK, size - off)
        part = f"{out}.part{off:#08x}"
        if not read_chunk(addr + off, n, part):
            raise SystemExit(f"FAILED: {addr + off:#x} ({n} B) after retries")
        with open(part, "rb") as fh:
            b = fh.read()
        assert len(b) == n, (part, len(b), n)
        data += b
        os.unlink(part)
        off += n
    with open(out, "wb") as fh:
        fh.write(data)
    assert os.path.getsize(out) == size, (out, os.path.getsize(out), size)
    return hashlib.sha256(data).hexdigest(), len(data)


if __name__ == "__main__":
    addr = int(sys.argv[1], 0)
    size = int(sys.argv[2], 0)
    out = sys.argv[3]
    h, n = read_region(addr, size, out)
    print(f"{out}: {n} B  sha256 {h}")
