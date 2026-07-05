# BLE Environmental Sensor Node — CLI Interface Specification

Version 1.0

## Transport

- UART, 115200 baud, 8N1 (native simulator: stdin/stdout).
- Commands are ASCII lines terminated by `\n` or `\r`.
- Maximum line length: 63 characters. Longer lines are discarded and answered
  with exactly one `ERR line too long`.
- Every command produces **exactly one** response line, terminated `\r\n`.

## Response convention

- Success: `OK` or `OK <payload>`
- Failure: `ERR <reason>`
- Payload fields use `NAME=value` tokens separated by single spaces, with unit
  suffixes: `C` (°C), `%` (RH), `hPa`, `lx`, `V`, `ms`.

This convention makes responses trivially machine-parsable (see `test/dut.py`).

## Commands

| Command | Response example | Notes |
|---|---|---|
| `ver` | `OK ESN v0.1.0` | Semantic version |
| `help` | `OK commands: ...` | Lists all commands |
| `read all` | `OK T=22.51C RH=44.0% P=1007.9hPa L=311lx VBAT=2.96V` | Fresh sample |
| `read temp` | `OK T=22.51C` | |
| `read hum` | `OK RH=44.0%` | |
| `read pres` | `OK P=1007.9hPa` | |
| `read light` | `OK L=311lx` | |
| `batt` | `OK VBAT=2.96V` | |
| `ble status` | `OK BLE=ADVERTISING` | `ADVERTISING` or `IDLE` |
| `selftest` | `OK BME280=PASS VEML7700=PASS VBAT=PASS` | `ERR ...` if any FAIL |
| `uptime` | `OK UPTIME=182734ms` | ms since boot |
| `sleep <s>` | `OK sleeping 30s` | 1–3600 s; enters STOP2 on hardware |
| `hang` | `OK hanging` | Test hook: hangs the task; watchdog resets |
| *(unknown)* | `ERR unknown command` | Device must remain responsive |

## Versioning

Breaking changes to this specification require a firmware MAJOR version bump
and a matching update to the test suite.
