/**
 * @file esn.h
 * @brief Core types and platform abstraction for the BLE Environmental Sensor Node.
 *
 * The core firmware is platform-independent. Each port (native simulator,
 * STM32WB55 target) provides an implementation of the esn_hal_* functions.
 */
#ifndef ESN_H
#define ESN_H

#include <stdint.h>
#include <stdbool.h>

#define ESN_FW_VERSION "0.1.0"

/** One complete measurement sample. */
typedef struct {
    float    temperature_c;   /**< Degrees Celsius */
    float    humidity_pct;    /**< Relative humidity, 0..100 % */
    float    pressure_hpa;    /**< Barometric pressure, hPa */
    float    light_lux;       /**< Ambient light, lux */
    float    battery_v;       /**< Battery voltage, volts */
    uint32_t timestamp_ms;    /**< Milliseconds since boot */
} esn_measurement_t;

/** Self-test result per subsystem. */
typedef struct {
    bool bme280_ok;
    bool veml7700_ok;
    bool battery_ok;
} esn_selftest_t;

/* ---- Platform abstraction (implemented by each port) ---- */

/** Read all sensors into @p m. Returns true on success. */
bool esn_hal_read_sensors(esn_measurement_t *m);

/** Probe each device on the bus. */
void esn_hal_selftest(esn_selftest_t *st);

/** Milliseconds since boot. */
uint32_t esn_hal_millis(void);

/** True if the BLE stack is currently advertising. */
bool esn_hal_ble_advertising(void);

/** Request low-power sleep for @p seconds (may be a no-op in simulation). */
void esn_hal_sleep(uint32_t seconds);

/** Write a NUL-terminated string to the CLI transport (UART / stdout). */
void esn_hal_cli_write(const char *s);

/** Deliberately hang the current task (watchdog test). */
void esn_hal_hang(void);

#endif /* ESN_H */
