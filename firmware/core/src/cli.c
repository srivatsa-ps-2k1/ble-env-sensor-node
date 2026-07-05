/**
 * @file cli.c
 * @brief Implementation of the BLE Environmental Sensor Node CLI (docs/cli-spec.md).
 */
#include "cli.h"
#include "esn.h"

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

static char s_line[CLI_MAX_LINE];
static size_t s_len = 0;
static bool s_discarding = false;  /* true while swallowing an oversized line */

/* ---- helpers ------------------------------------------------------- */

static void reply(const char *s)
{
    esn_hal_cli_write(s);
    esn_hal_cli_write("\r\n");
}

static void replyf(const char *fmt, ...)
{
    char buf[128];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    reply(buf);
}

/* Case-insensitive comparison of the full command string. */
static bool cmd_is(const char *line, const char *cmd)
{
    while (*cmd) {
        if (tolower((unsigned char)*line) != tolower((unsigned char)*cmd))
            return false;
        line++; cmd++;
    }
    return *line == '\0';
}

/* ---- command handlers ---------------------------------------------- */

static void cmd_ver(void)
{
    replyf("OK ESN v%s", ESN_FW_VERSION);
}

static void cmd_read(const char *what)
{
    esn_measurement_t m;
    if (!esn_hal_read_sensors(&m)) {
        reply("ERR sensor read failed");
        return;
    }
    if (cmd_is(what, "all")) {
        replyf("OK T=%.2fC RH=%.1f%% P=%.1fhPa L=%.0flx VBAT=%.2fV",
               m.temperature_c, m.humidity_pct, m.pressure_hpa,
               m.light_lux, m.battery_v);
    } else if (cmd_is(what, "temp")) {
        replyf("OK T=%.2fC", m.temperature_c);
    } else if (cmd_is(what, "hum")) {
        replyf("OK RH=%.1f%%", m.humidity_pct);
    } else if (cmd_is(what, "pres")) {
        replyf("OK P=%.1fhPa", m.pressure_hpa);
    } else if (cmd_is(what, "light")) {
        replyf("OK L=%.0flx", m.light_lux);
    } else {
        reply("ERR unknown channel");
    }
}

static void cmd_batt(void)
{
    esn_measurement_t m;
    if (!esn_hal_read_sensors(&m)) {
        reply("ERR sensor read failed");
        return;
    }
    replyf("OK VBAT=%.2fV", m.battery_v);
}

static void cmd_ble_status(void)
{
    replyf("OK BLE=%s", esn_hal_ble_advertising() ? "ADVERTISING" : "IDLE");
}

static void cmd_selftest(void)
{
    esn_selftest_t st;
    esn_hal_selftest(&st);
    replyf("%s BME280=%s VEML7700=%s VBAT=%s",
           (st.bme280_ok && st.veml7700_ok && st.battery_ok) ? "OK" : "ERR",
           st.bme280_ok    ? "PASS" : "FAIL",
           st.veml7700_ok  ? "PASS" : "FAIL",
           st.battery_ok   ? "PASS" : "FAIL");
}

static void cmd_uptime(void)
{
    replyf("OK UPTIME=%lums", (unsigned long)esn_hal_millis());
}

static void cmd_sleep(const char *arg)
{
    unsigned long secs = 0;
    if (sscanf(arg, "%lu", &secs) != 1 || secs == 0 || secs > 3600) {
        reply("ERR usage: sleep <1..3600>");
        return;
    }
    replyf("OK sleeping %lus", secs);
    esn_hal_sleep((uint32_t)secs);
}

static void cmd_help(void)
{
    reply("OK commands: ver | read all|temp|hum|pres|light | batt | "
          "ble status | selftest | uptime | sleep <s> | hang | help");
}

/* ---- dispatcher ----------------------------------------------------- */

void cli_execute(const char *line)
{
    /* Skip leading whitespace. */
    while (*line == ' ' || *line == '\t') line++;

    if (*line == '\0')                  { return; }               /* ignore empty */
    else if (cmd_is(line, "ver"))       { cmd_ver(); }
    else if (strncmp(line, "read ", 5) == 0) { cmd_read(line + 5); }
    else if (cmd_is(line, "batt"))      { cmd_batt(); }
    else if (cmd_is(line, "ble status")){ cmd_ble_status(); }
    else if (cmd_is(line, "selftest"))  { cmd_selftest(); }
    else if (cmd_is(line, "uptime"))    { cmd_uptime(); }
    else if (strncmp(line, "sleep ", 6) == 0) { cmd_sleep(line + 6); }
    else if (cmd_is(line, "hang"))      { reply("OK hanging"); esn_hal_hang(); }
    else if (cmd_is(line, "help"))      { cmd_help(); }
    else                                { reply("ERR unknown command"); }
}

void cli_feed_char(char c)
{
    if (c == '\n' || c == '\r') {
        if (s_discarding) {
            /* End of an oversized line: report once, then resync. */
            s_discarding = false;
            reply("ERR line too long");
        } else if (s_len > 0) {
            s_line[s_len] = '\0';
            cli_execute(s_line);
        }
        s_len = 0;
        return;
    }
    if (s_discarding) {
        return;  /* swallow the rest of the oversized line */
    }
    if (s_len < CLI_MAX_LINE - 1) {
        s_line[s_len++] = c;
    } else {
        /* Overflow: switch to discard mode until the newline arrives,
         * so exactly one ERR is emitted per oversized line and the
         * request/response protocol stays in sync. */
        s_discarding = true;
        s_len = 0;
    }
}
