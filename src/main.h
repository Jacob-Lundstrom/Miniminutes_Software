#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <nrfx_timer.h>

#include <nrfx_clock.h>
#include <nrfx_rtc.h>

#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/sys/printk.h>

#define DISPLAY_NOTHING_WHILE_CHARGING 0
#define DISPLAY_PERCENT_WHILE_CHARGING 1
#define DISPLAY_VOLTAGE_WHILE_CHARGING 2
#define DISPLAY_TIME_WHILE_CHARGING 3
#define DISPLAY_TIME_AND_PERCENT_WHILE_CHARGING 4
// Add more display charging modes here

static bool need_to_check_input;
static bool show_time;
static bool show_percent;
static bool show_voltage;
static bool always_on;
// static bool always_on_while_charging; // Deprecated
static uint8_t charging_display_mode;
static bool is_charging;
static bool display_thread_enabled;


static int battery_mv;
static int battery_p;
static int reset_ble_required;

int THREAD_main_DEV(void);
int THREAD_main(void);
int THREAD_battery_monitor(void);
int THREAD_display(void);

void resume_display(void);
void continue_showing_time(void);
void continue_showing_battery_percent(void);
void continue_showing_battery_voltage(void);

void set_time(uint32_t time);
void set_military_time(bool status);

void IMU_wakeup_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
void PWR_wakeup_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
void display_timeout_isr(struct k_timer *dummy);


void simulate_IMU_interrupt(void);

void reset_BLE(void);

void start_BLE_thread(void);

void set_always_on(bool state);
void set_display_mode_while_charging(uint8_t state);