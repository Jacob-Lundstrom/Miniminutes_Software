#ifndef IMU_H
#define IMU_H
#include "main.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

// CHOOSE ONE FOR THE CORRECT IMU
// #define __LIS2DW12
#define __LIS2DUX12

#define INT1_NODE DT_ALIAS(int1)
static const struct gpio_dt_spec INT1 = GPIO_DT_SPEC_GET(INT1_NODE, gpios);

#ifdef __LIS2DW12
#define I2C0_IMU_NODE DT_NODELABEL(lis2dw12)
static const struct i2c_dt_spec imu_i2c = I2C_DT_SPEC_GET(I2C0_IMU_NODE);
#endif

#ifdef __LIS2DUX12

#define FF_IA_ALL_MASK              0b0000001
#define WU_IA_ALL_MASK              0b0000010
#define SINGLE_TAP_ALL_MASK         0b0000100
#define DOUBLE_TAP_ALL_MASK         0b0001000
#define TRIPLE_TAP_ALL_MASK         0b0010000
#define D6D_IA_ALL_MASK             0b0100000
#define SLEEP_CHANGE_IA_ALL_MASK    0b1000000

#define I2C0_IMU_NODE DT_NODELABEL(lis2dux12)
static const struct i2c_dt_spec imu_i2c = I2C_DT_SPEC_GET(I2C0_IMU_NODE);

#endif

void IMU_init(void);
uint8_t write_to_IMU(uint8_t reg, uint8_t val);
uint8_t read_from_IMU(uint8_t reg);

#endif