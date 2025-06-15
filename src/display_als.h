#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#include "main.h"
#include "imu.h"


#define I2C0_DISPLAY_ALS_NODE DT_NODELABEL(tsl2584tsv)
static const struct i2c_dt_spec display_als_i2c = I2C_DT_SPEC_GET(I2C0_DISPLAY_ALS_NODE);

#define DISPLAY_ALS_REG_CONTROL             0x00
#define DISPLAY_ALS_REG_TIMING              0x01
#define DISPLAY_ALS_REG_INTERRUPT           0x02
#define DISPLAY_ALS_REG_THL_LOW             0x03
#define DISPLAY_ALS_REG_THL_HIGH            0x04
#define DISPLAY_ALS_REG_THH_LOW             0x05
#define DISPLAY_ALS_REG_THH_HIGH            0x06
#define DISPLAY_ALS_REG_ANALOG              0x07
#define DISPLAY_ALS_REG_ID                  0x12
#define DISPLAY_ALS_REG_DATA0_LOW           0x14
#define DISPLAY_ALS_REG_DATA0_HIGH          0x15
#define DISPLAY_ALS_REG_DATA1_LOW           0x16
#define DISPLAY_ALS_REG_DATA1_HIGH          0x17
#define DISPLAY_ALS_REG_TIMER_LOW           0x18
#define DISPLAY_ALS_REG_TIMER_HIGH          0x19
#define DISPLAY_ALS_REG_ID2                 0x1E


void Display_ALS_init(void);
uint8_t write_to_Display_ALS(uint8_t reg, uint8_t val);
uint8_t read_from_Display_ALS(uint8_t reg);
uint8_t command_Display_ALS(uint8_t COMMAND);
void enable_Display_ALS_interrupt(void);
int32_t Display_ALS_get_brightness(void);
uint8_t Display_ALS_set_gain(uint8_t gain);
uint8_t Display_ALS_check_ID(void);
uint8_t Display_ALS_set_exposure_ms(float exposure_ms);
uint8_t Display_ALS_set_gain(uint8_t gain);