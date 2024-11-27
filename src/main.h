#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <nrfx_timer.h>

#include <nrfx_clock.h>
#include <nrfx_rtc.h>

#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/sys/printk.h>

void IMU_wakeup_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins);
