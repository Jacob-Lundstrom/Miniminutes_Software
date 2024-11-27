#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <nrfx_timer.h>

#include <nrfx_clock.h>
#include <nrfx_rtc.h>

#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/sys/printk.h>


static uint32_t SYSTEM_TIME_SECONDS;
static bool show_time;
static bool show_percent;
static bool show_voltage;


static int battery_mv;
static int battery_p;

void IMU_wakeup_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

void display_timeout_isr(struct k_timer *dummy);

void resume_main_thread(void);
void continue_showing_time(void);
void continue_showing_battery_percent(void);
void continue_showing_battery_voltage(void);

void set_time(uint32_t time);

void set_military_time(bool status);