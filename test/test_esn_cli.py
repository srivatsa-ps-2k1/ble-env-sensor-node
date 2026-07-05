"""BLE Environmental Sensor Node CLI verification suite.

Each test's docstring names the requirement(s) it verifies
(docs/requirements.md) — requirements-to-test traceability, the same
practice used in regulated device validation.

Run against the simulator (default) or real hardware (``--port ...``).
"""
import re

import pytest

from dut import SimDut


# ---------------------------------------------------------------- identity

def test_firmware_version_format(dut):
    """REQ-01: The device shall report its firmware version via the CLI.

    The version string must follow semantic versioning (MAJOR.MINOR.PATCH).
    """
    r = dut.send_command("ver")
    assert r.ok
    assert re.search(r"ESN v\d+\.\d+\.\d+", r.raw)


def test_help_lists_commands(dut):
    """REQ-02: The CLI shall provide a help command listing all commands."""
    r = dut.send_command("help")
    assert r.ok
    for cmd in ("ver", "read", "selftest", "sleep"):
        assert cmd in r.raw


# ---------------------------------------------------------------- selftest

def test_selftest_passes(dut):
    """REQ-10: A self-test shall verify communication with every sensor."""
    r = dut.send_command("selftest")
    assert r.ok
    assert r.field("BME280") == "PASS"
    assert r.field("VEML7700") == "PASS"
    assert r.field("VBAT") == "PASS"


def test_selftest_reports_failed_sensor(sim_only):
    """REQ-11: The self-test shall identify WHICH device failed.

    Simulator-only: injects a BME280 fault via ESN_SIM_FAULT.
    """
    d = SimDut(env={"ESN_SIM_FAULT": "bme280"})
    try:
        r = d.send_command("selftest")
        assert r.err
        assert r.field("BME280") == "FAIL"
        assert r.field("VEML7700") == "PASS"
    finally:
        d.close()


# ---------------------------------------------------------------- readings

def test_read_all_plausibility(dut):
    """REQ-03/04/05a: Measurements shall be within physically plausible ranges.

    Windows are wide on purpose: they catch driver bugs (wrong register,
    byte-order, missing calibration) rather than judging the environment.
    """
    r = dut.send_command("read all")
    assert r.ok
    assert -20.0 <= r.field_float("T") <= 60.0, "temperature implausible"
    assert 0.0 <= r.field_float("RH") <= 100.0, "humidity out of physical range"
    assert 900.0 <= r.field_float("P") <= 1100.0, "pressure implausible"
    assert r.field_float("L") >= 0.0, "negative lux impossible"
    assert 1.8 <= r.field_float("VBAT") <= 4.5, "battery voltage implausible"


@pytest.mark.parametrize(
    "channel,field",
    [("temp", "T"), ("hum", "RH"), ("pres", "P"), ("light", "L")],
)
def test_read_single_channels(dut, channel, field):
    """REQ-06: Each measurement shall be readable individually."""
    r = dut.send_command(f"read {channel}")
    assert r.ok
    r.field_float(field)  # raises if missing / non-numeric


def test_battery_voltage(dut):
    """REQ-07: The device shall report battery voltage."""
    r = dut.send_command("batt")
    assert r.ok
    assert 1.8 <= r.field_float("VBAT") <= 4.5


def test_readings_vary_between_samples(dut):
    """REQ-08: Consecutive reads shall return fresh samples (not a stuck
    cached value) — catches a classic I2C driver bug."""
    values = {dut.send_command("read all").raw for _ in range(5)}
    assert len(values) > 1, "5 identical samples suggests a stuck sensor read"


# ---------------------------------------------------------------- radio

def test_ble_status_reports_state(dut):
    """REQ-09: The device shall report its BLE state via the CLI."""
    r = dut.send_command("ble status")
    assert r.ok
    assert r.field("BLE") in ("ADVERTISING", "IDLE")


# ---------------------------------------------------------------- robustness

def test_unknown_command_returns_err(dut):
    """REQ-12: Unknown commands shall return an ERR response, never crash."""
    r = dut.send_command("frobnicate")
    assert r.err
    assert dut.is_alive()


def test_oversized_line_is_rejected(dut):
    """REQ-12: Oversized input shall be rejected gracefully (no overflow)."""
    r = dut.send_command("x" * 200)
    assert r.err
    # Device must still work afterwards:
    assert dut.send_command("ver").ok


def test_sleep_argument_validation(dut):
    """REQ-13: The sleep command shall validate its argument."""
    assert dut.send_command("sleep 0").err
    assert dut.send_command("sleep abc").err
    assert dut.send_command("sleep 2").ok


def test_hang_triggers_watchdog_recovery(sim_only):
    """REQ-14: A hung task shall be recovered by the watchdog.

    Simulator: the process exits with code 42 (stand-in for an IWDG reset).
    On hardware, this test instead watches for the reset banner.
    """
    d = SimDut()
    try:
        r = d.send_command("hang")
        assert r.ok
        d.proc.wait(timeout=2)
        assert d.returncode == 42
    finally:
        d.close()
