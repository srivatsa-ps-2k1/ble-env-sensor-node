"""Device-under-test (DUT) driver for the BLE Environmental Sensor Node.

Two interchangeable transports expose the same send_command() API:

* SimDut    -- spawns the native simulator binary and talks over stdin/stdout.
               Used for hardware-free development and CI.
* SerialDut -- talks to the real board's UART CLI via pyserial.
               Used for hardware-in-the-loop runs (``pytest --port /dev/ttyACM0``).

Response convention (docs/cli-spec.md): every reply is one line ending in
CRLF, starting with ``OK`` or ``ERR``.
"""
from __future__ import annotations

import os
import re
import subprocess
from dataclasses import dataclass


@dataclass
class Response:
    raw: str

    @property
    def ok(self) -> bool:
        return self.raw.startswith("OK")

    @property
    def err(self) -> bool:
        return self.raw.startswith("ERR")

    def field(self, name: str) -> str:
        """Extract 'name=value' payload fields, e.g. field('T') -> '22.51C'."""
        m = re.search(rf"{re.escape(name)}=([^\s]+)", self.raw)
        if not m:
            raise KeyError(f"field {name!r} not in response: {self.raw!r}")
        return m.group(1)

    def field_float(self, name: str) -> float:
        """Extract a numeric field, stripping unit suffixes (C, %, hPa, lx, V)."""
        value = self.field(name)
        m = re.match(r"[-+]?\d+(?:\.\d+)?", value)
        if not m:
            raise ValueError(f"field {name!r} is not numeric: {value!r}")
        return float(m.group(0))


class SimDut:
    """Runs the native simulator binary as a subprocess."""

    BINARY = os.path.join(
        os.path.dirname(__file__), "..", "firmware", "ports", "native", "esn-sim"
    )

    def __init__(self, env: dict[str, str] | None = None):
        full_env = os.environ.copy()
        if env:
            full_env.update(env)
        self.proc = subprocess.Popen(
            [self.BINARY],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            text=True,
            bufsize=1,
            env=full_env,
        )
        banner = self.proc.stdout.readline().strip()
        assert "ESN-SIM ready" in banner, f"unexpected banner: {banner!r}"

    def send_command(self, cmd: str, timeout: float = 2.0) -> Response:
        assert self.proc.poll() is None, "simulator process has exited"
        self.proc.stdin.write(cmd + "\n")
        self.proc.stdin.flush()
        line = self.proc.stdout.readline().strip()
        return Response(line)

    def is_alive(self) -> bool:
        return self.proc.poll() is None

    @property
    def returncode(self):
        return self.proc.poll()

    def close(self):
        if self.proc.poll() is None:
            self.proc.stdin.close()
            self.proc.terminate()
            self.proc.wait(timeout=5)


class SerialDut:
    """Talks to real hardware over a serial port (lazy pyserial import)."""

    def __init__(self, port: str, baud: int = 115200, timeout: float = 2.0):
        import serial  # only needed for HIL runs

        self.ser = serial.Serial(port, baud, timeout=timeout)
        self.ser.reset_input_buffer()

    def send_command(self, cmd: str, timeout: float = 2.0) -> Response:
        self.ser.write((cmd + "\n").encode())
        line = self.ser.readline().decode(errors="replace").strip()
        return Response(line)

    def is_alive(self) -> bool:
        return self.ser.is_open

    def close(self):
        self.ser.close()
