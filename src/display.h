#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)
#define LED4_NODE DT_ALIAS(led4)

#define I2C0_DISPLAY_NODE DT_NODELABEL(fxl6408umx)


static const struct gpio_dt_spec S1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec S2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec S3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec S4 = GPIO_DT_SPEC_GET(LED4_NODE, gpios);

static const struct i2c_dt_spec display_i2c = I2C_DT_SPEC_GET(I2C0_DISPLAY_NODE);

uint8_t writeToDisplay(uint8_t reg, uint8_t val);
uint8_t readFromDisplay(uint8_t reg);
int Display_init();
void enableSegment(int segment);
void disableSegments();
void displayNone();
void displayNumberOnSegment(uint8_t num, int segment);
void displayColon();
void displayTime(int num1, int num2, bool colon, int num3, int num4);
void displayOff();
void display_time_seconds(int current_time_seconds);
void display_time_seconds_mil(int current_time_seconds, bool military_time);
void display_battery_voltage_mv(int mv);
void display_error(void);
void display_credits(void);
void display_percent(int percent);