# BLE Environmental Sensor Node — Requirements Specification

Version 1.0 · Status: Draft

Verification methods: **T** = automated test (see `/test`), **M** = measurement,
**I** = inspection, **D** = demonstration.

## Functional

| ID | Requirement | Verify |
|----|-------------|--------|
| REQ-01 | The device shall report its firmware version (semantic versioning) via the CLI. | T |
| REQ-02 | The CLI shall provide a `help` command listing all available commands. | T |
| REQ-03 | The device shall measure ambient temperature (−10…+50 °C, ±1 °C) and relative humidity (0–100 %, ±3 %). | T, M |
| REQ-04 | The device shall measure barometric pressure (300–1100 hPa, ±1 hPa). | T, M |
| REQ-05 | The device shall measure ambient light (0–120 000 lx). | T, M |
| REQ-05a | All reported measurements shall lie within physically plausible ranges. | T |
| REQ-06 | Each measurement channel shall be individually readable via the CLI. | T |
| REQ-07 | The device shall measure and report its own battery voltage. | T, M |
| REQ-08 | Consecutive sensor reads shall return fresh samples, not cached values. | T |
| REQ-09 | The device shall expose measurements over a BLE GATT service and report BLE state via the CLI. | T, D |
| REQ-10 | A self-test shall verify communication with every sensor device. | T |
| REQ-11 | The self-test shall identify which specific device failed. | T |

## Robustness

| ID | Requirement | Verify |
|----|-------------|--------|
| REQ-12 | Malformed or oversized CLI input shall produce an `ERR` response and never crash or desynchronize the device. | T |
| REQ-13 | Commands with arguments shall validate them and reject invalid values. | T |
| REQ-14 | A hung task shall be detected and recovered by the independent watchdog within 3 s. | T, D |

## Power (hardware target only)

| ID | Requirement | Verify |
|----|-------------|--------|
| REQ-15 | Standby (STOP2) current shall be below 20 µA. | M |
| REQ-16 | The device shall operate from a CR2032 coin cell or 1S LiPo (2.0–4.4 V input). | M, I |

## Physical

| ID | Requirement | Verify |
|----|-------------|--------|
| REQ-17 | The PCB shall not exceed 40 × 40 mm and shall expose SWD and UART headers. | I |

> Note: REQ-15/16/17 cannot be verified in simulation; they are validated
> during board bring-up (see `docs/bringup-log.md`).
