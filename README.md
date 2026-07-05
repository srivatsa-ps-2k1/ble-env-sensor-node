# BLE Environmental Sensor Node — BLE Environmental Sensor Node

A coin-cell-powered wireless sensor node (temperature, humidity, pressure,
ambient light) built around the STM32WB55, with a **simulation-first,
test-driven** development workflow: the firmware core runs identically on a
PC simulator and on the target, and a requirement-traceable pytest suite
verifies the CLI contract on both.

> **Status:** firmware core + native simulator + test framework working.
> KiCad hardware design and STM32WB55 port in progress — see the
> [roadmap](#roadmap).

## Why this project

Commercial products in smart-home sensing, cold-chain logging, and building
automation share this exact architecture: low-power MCU + I²C sensors +
2.4 GHz radio + a testable serial interface. BLE Environmental Sensor Node is a from-scratch,
fully-documented implementation of that pattern, covering the complete
lifecycle: requirements → schematic/PCB → RTOS firmware → automated
validation.

## Architecture

```
                 ┌───────────────────────────────────────────┐
                 │            firmware/core  (portable C11)   │
                 │   cli.c ── command parser (docs/cli-spec)  │
                 │   esn.h ── HAL abstraction interface      │
                 └───────────────┬───────────────┬───────────┘
                                 │               │
                  ┌──────────────┴───┐   ┌───────┴───────────────┐
                  │ ports/native      │   │ ports/stm32wb         │
                  │ PC simulator      │   │ FreeRTOS, 4 tasks,    │
                  │ (stdin/stdout,    │   │ BLE GATT, STOP2,      │
                  │ fault injection)  │   │ IWDG  (in progress)   │
                  └──────────┬───────┘   └───────────┬───────────┘
                             │                       │
                  ┌──────────┴───────────────────────┴───────────┐
                  │ test/  — pytest suite (same tests, both DUTs)│
                  │   default: simulator   |   --port /dev/ttyX  │
                  └──────────────────────────────────────────────┘
```

Planned FreeRTOS task layout on target: `SensorTask` (I²C acquisition →
queue), `BleTask` (GATT updates), `CliTask` (UART line CLI), and
`HousekeepingTask` (battery ADC, watchdog, STOP2 entry).

## Quick start (no hardware needed)

```bash
# Build the simulator
cd firmware/ports/native && make

# Talk to it interactively
./esn-sim
ver
read all
selftest

# Run the full test suite
cd ../../../test
pip install pytest pytest-html
pytest -v --html=report.html --self-contained-html
```

To run the same suite against real hardware later:

```bash
pip install pyserial
pytest --port /dev/ttyACM0 -v
```

Fault injection in the simulator (negative testing):

```bash
ESN_SIM_FAULT=bme280 ./esn-sim   # selftest now reports BME280=FAIL
```

## Testing philosophy

- Every test names the requirement it verifies (`docs/requirements.md`) —
  requirements-to-test traceability as used in regulated device validation.
- One CLI contract, two DUTs: the simulator enables fast CI feedback; the
  serial transport reuses identical tests for hardware-in-the-loop runs.
- CI (GitHub Actions) builds the simulator, runs the suite, uploads the HTML
  report, and cross-compiles the core for Cortex-M4 on every push.
- The suite has already caught one real bug during development: the CLI's
  original overflow handling emitted multiple `ERR` lines for one oversized
  input, desynchronizing the request/response protocol. Fixed with a
  discard-until-newline state (see `cli.c` history).

## Repository layout

```
docs/        requirements.md · cli-spec.md · bringup-log.md
firmware/
  core/      portable C11 core (CLI parser, HAL interface)
  ports/
    native/  PC simulator (gcc + Makefile)
    stm32wb/ STM32WB55 port skeleton (CubeIDE integration notes)
hardware/    KiCad project (Phases 1–2 — see hardware/README.md)
test/        pytest suite + DUT drivers (simulator & serial)
```

## Roadmap

- [x] Requirements & CLI specification
- [x] Portable firmware core + native simulator
- [x] pytest framework with fault injection, HTML reports, CI
- [x] Core cross-compiles for Cortex-M4 (CI job)
- [ ] KiCad schematic & PCB (STM32WB55, BME280, VEML7700, chip antenna)
- [ ] STM32WB55 port: FreeRTOS tasks, sensor drivers, BLE GATT service
- [ ] Board bring-up (`docs/bringup-log.md`) & standby-current measurement
- [ ] Hardware-in-the-loop test run + report

## License

MIT (firmware & tests). Hardware design files: CERN-OHL-P (when published).
