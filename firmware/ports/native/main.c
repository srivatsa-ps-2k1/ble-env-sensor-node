/**
 * @file main.c
 * @brief Native (PC) simulator port of the BLE Environmental Sensor Node firmware.
 *
 * Runs the exact same core CLI code as the STM32 target, but on your PC:
 * stdin/stdout replace the UART. This lets the pytest suite in /test run
 * against the firmware with zero hardware ("simulation-first" workflow),
 * and lets CI exercise the CLI on every push.
 *
 * Simulated sensor values are plausible and slightly noisy so range-checking
 * tests are meaningful. Set the environment variable ESN_SIM_FAULT=bme280
 * (or veml7700, battery) to simulate a failed device for negative tests.
 *
 * Build:  gcc -Wall -Wextra -std=c11 -I../../core/include \
 *             ../../core/src/cli.c main.c -o esn-sim -lm
 * Run:    ./esn-sim        (type commands, e.g. "ver", "read all")
 */
#include "esn.h"
#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

static struct timespec s_boot;

static bool fault(const char *name)
{
    const char *f = getenv("ESN_SIM_FAULT");
    return f && strcmp(f, name) == 0;
}

/* Small deterministic-ish noise source. */
static float noise(float amplitude)
{
    return amplitude * ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f;
}

uint32_t esn_hal_millis(void)
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (uint32_t)((now.tv_sec - s_boot.tv_sec) * 1000
                    + (now.tv_nsec - s_boot.tv_nsec) / 1000000);
}

bool esn_hal_read_sensors(esn_measurement_t *m)
{
    if (fault("bme280") || fault("veml7700"))
        return false;

    m->temperature_c = 22.5f + noise(1.5f);
    m->humidity_pct  = 45.0f + noise(8.0f);
    m->pressure_hpa  = 1008.0f + noise(4.0f);
    m->light_lux     = 320.0f + noise(150.0f);
    if (m->light_lux < 0) m->light_lux = 0;
    m->battery_v     = 2.95f + noise(0.05f);
    m->timestamp_ms  = esn_hal_millis();
    return true;
}

void esn_hal_selftest(esn_selftest_t *st)
{
    st->bme280_ok   = !fault("bme280");
    st->veml7700_ok = !fault("veml7700");
    st->battery_ok  = !fault("battery");
}

bool esn_hal_ble_advertising(void)
{
    /* The native build has no radio; report ADVERTISING so the CLI
     * contract can be tested. The STM32 port returns real stack state. */
    return true;
}

void esn_hal_sleep(uint32_t seconds)
{
    /* Compress simulated sleep so tests stay fast: 1 s -> 10 ms. */
    struct timespec ts = { .tv_sec = 0, .tv_nsec = (long)seconds * 10000000L };
    nanosleep(&ts, NULL);
}

void esn_hal_cli_write(const char *s)
{
    fputs(s, stdout);
    fflush(stdout);
}

void esn_hal_hang(void)
{
    /* Simulate a hung task. On the STM32 target the IWDG resets the MCU;
     * here we just exit with a distinctive code the test can observe. */
    exit(42);
}

int main(void)
{
    clock_gettime(CLOCK_MONOTONIC, &s_boot);
    srand((unsigned)time(NULL));

    /* Banner (single line, parsable). */
    esn_hal_cli_write("ESN-SIM ready\r\n");

    int c;
    while ((c = getchar()) != EOF) {
        cli_feed_char((char)c);
    }
    return 0;
}
