#include "main.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#define INT1_NODE DT_ALIAS(int1)
static const struct gpio_dt_spec INT1 = GPIO_DT_SPEC_GET(INT1_NODE, gpios);

#define I2C0_IMU_NODE DT_NODELABEL(lis2dw12tr)

static const struct i2c_dt_spec imu_i2c = I2C_DT_SPEC_GET(I2C0_IMU_NODE);

uint8_t writeToIMU(uint8_t reg, uint8_t val);
uint8_t readFromIMU(uint8_t reg);
void enableInt1();