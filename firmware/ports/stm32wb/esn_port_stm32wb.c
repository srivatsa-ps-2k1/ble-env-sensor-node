/**
 * @file esn_port_stm32wb.c
 * @brief STM32WB55 port of the BLE Environmental Sensor Node HAL — integration skeleton.
 *
 * HOW TO USE THIS FILE
 * --------------------
 * 1. Generate a base project in STM32CubeIDE for your board
 *    (P-NUCLEO-WB55 first, later your custom PCB):
 *      - Middleware: FreeRTOS (CMSIS-RTOS2), STM32_WPAN (BLE, custom template)
 *      - Peripherals: I2C1, USART1 (115200 8N1, RX interrupt), ADC1, IWDG,
 *        RTC (for STOP2 wake-up), LPTIM optional.
 * 2. Add firmware/core/include to the include path and firmware/core/src/cli.c
 *    plus this file to the build.
 * 3. Fill in every TODO below. Each TODO maps to a concrete step in the
 *    project build guide (Phase 3).
 * 4. Create the four tasks in app_freertos.c and wire the UART RX interrupt
 *    to cli_feed_char() via a queue (never call the CLI from the ISR).
 *
 * This file intentionally does not compile standalone — it depends on
 * CubeIDE-generated code (main.h, HAL handles, BLE app headers).
 */
#include "esn.h"
#include "cli.h"

/* #include "main.h"          -- CubeIDE generated */
/* #include "app_ble.h"       -- CubeIDE generated (STM32_WPAN) */
/* extern I2C_HandleTypeDef  hi2c1;  */
/* extern UART_HandleTypeDef huart1; */
/* extern ADC_HandleTypeDef  hadc1;  */

#define BME280_ADDR    (0x76 << 1)
#define VEML7700_ADDR  (0x10 << 1)

uint32_t esn_hal_millis(void)
{
    /* TODO: return HAL_GetTick(); */
    return 0;
}

bool esn_hal_read_sensors(esn_measurement_t *m)
{
    /* TODO Phase 3, step 2:
     *  - Port the official Bosch BME280 driver (github.com/BoschSensortec/BME280_driver)
     *    with an I2C read/write shim using HAL_I2C_Mem_Read/Write on hi2c1.
     *  - Write the VEML7700 driver from its datasheet (registers 0x00 config,
     *    0x04 ALS output). It is a great from-scratch driver exercise.
     *  - Read VBAT through the ADC divider; convert counts -> volts using
     *    VREFINT calibration for accuracy.
     */
    (void)m;
    return false;
}

void esn_hal_selftest(esn_selftest_t *st)
{
    /* TODO Phase 3, step 7:
     *  st->bme280_ok   = HAL_I2C_IsDeviceReady(&hi2c1, BME280_ADDR,  2, 10) == HAL_OK
     *                    && bme280_chip_id() == 0x60;
     *  st->veml7700_ok = HAL_I2C_IsDeviceReady(&hi2c1, VEML7700_ADDR, 2, 10) == HAL_OK;
     *  st->battery_ok  = (vbat > 2.0f && vbat < 4.4f);
     */
    st->bme280_ok = st->veml7700_ok = st->battery_ok = false;
}

bool esn_hal_ble_advertising(void)
{
    /* TODO Phase 3, step 4: query your BLE app state machine
     * (e.g. return APP_BLE_Get_Server_Connection_Status() == APP_BLE_LP_ADV). */
    return false;
}

void esn_hal_sleep(uint32_t seconds)
{
    /* TODO Phase 3, step 6:
     *  - Configure RTC wake-up timer for <seconds>.
     *  - Enter STOP2 via HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI).
     *  - On wake: restore clocks (SystemClock_Config()).
     *  NOTE: with FreeRTOS prefer tickless idle integration instead of a
     *  blocking call; see AN5289 for WB-specific low-power + BLE coexistence.
     */
    (void)seconds;
}

void esn_hal_cli_write(const char *s)
{
    /* TODO: HAL_UART_Transmit(&huart1, (uint8_t *)s, strlen(s), 100);
     * For higher throughput use DMA TX with a ring buffer. */
    (void)s;
}

void esn_hal_hang(void)
{
    /* Watchdog demonstration: stop refreshing the IWDG and spin.
     * The IWDG (configured ~2 s timeout in HousekeepingTask) resets the MCU;
     * the test framework detects the reset banner. */
    for (;;) { /* deliberate hang */ }
}
