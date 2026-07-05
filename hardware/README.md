# Hardware

KiCad project for the BLE Environmental Sensor Node PCB — **to be designed** (Phases 1–2
of the build guide).

Planned contents:
- `esn.kicad_pro` / `esn.kicad_sch` / `esn.kicad_pcb`
- `bom.csv`, exported Gerbers in `fab/`
- `schematic.pdf` render for the README

Key design references:
- ST AN5165 — STM32WB hardware development guidelines (crystals, decoupling, RF matching)
- Antenna datasheet reference layout (keep-out area is mandatory)
- LDO datasheet for input/output capacitor requirements

Design notes to carry over from the build guide:
- Keep the BME280 away from the LDO (self-heating skews temperature).
- Test points on SDA/SCL; SWD + UART headers per REQ-17.
- Battery-sense divider with high-value resistors (µA budget).
