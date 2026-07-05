"""pytest fixtures for the BLE Environmental Sensor Node test suite.

Default: tests run against the native simulator (no hardware required).
Hardware-in-the-loop: pass ``--port`` to run the same tests on a real board:

    pytest --port /dev/ttyACM0 --html=report.html --self-contained-html
"""
import pytest

from dut import SimDut, SerialDut


def pytest_addoption(parser):
    parser.addoption(
        "--port",
        action="store",
        default=None,
        help="Serial port of a real BLE Environmental Sensor Node board (e.g. /dev/ttyACM0). "
             "If omitted, tests run against the native simulator.",
    )


@pytest.fixture(scope="session")
def hw_port(request):
    return request.config.getoption("--port")


@pytest.fixture()
def dut(hw_port):
    """A fresh DUT per test: simulator subprocess, or the real board."""
    d = SerialDut(hw_port) if hw_port else SimDut()
    yield d
    d.close()


@pytest.fixture()
def sim_only(hw_port):
    """Skip tests that only make sense against the simulator."""
    if hw_port:
        pytest.skip("simulator-only test (fault injection / process exit)")
