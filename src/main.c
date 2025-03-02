/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// THIS PROGRAM REQUIRES nRF Connect SDK v2.6.1
// I don't think that the toolchain version matters.


#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/adc.h>
#include <nrfx_timer.h>

#include <nrfx_clock.h>
#include <nrfx_rtc.h>

#include <zephyr/device.h>
#include <zephyr/sys/printk.h>

#include <zephyr/kernel/thread_stack.h>

#include <zephyr/sys/poweroff.h>
#include <hal/nrf_power.h>

#include "main.h"
#include "display.h"
#include "imu.h"
#include "bluetooth.h"
#include "pwr.h"
#include "hr.h"
#include "rtc.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(smp_sample);

#define ALARM_CHANNEL_ID 0
#define TIMER DT_NODELABEL(rtc0)

static struct k_timer display_timeout;
static struct k_timer clock_increment_timer;

const k_tid_t thread_main_id;
#define MAIN_STACKSIZE 512
#define PRIORITY 7

static bool military_time = false;

#define TIMER_DEVICE_NODE DT_NODELABEL(timer0)

K_THREAD_DEFINE(thread_main_id, MAIN_STACKSIZE, thread_main_DEV, NULL, NULL, NULL,
		PRIORITY, 0, 0);

// K_THREAD_DEFINE(ble_thread_id, 2 * BLE_STACKSIZE, BLE_init, NULL, NULL, NULL,
// 		PRIORITY, 0, 0);




void IMU_wakeup_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
	if (show_time || show_percent || show_voltage){
		return;
	} else {
		continue_showing_time();
	}
}

void display_timeout_isr(struct k_timer *dummy) {
	if (!(always_on || (always_on_while_charging && is_charging))) {
		show_time = false;
		show_percent = false;
		show_voltage = false;
	}
	k_timer_stop(&display_timeout);
}

void time_increment_isr(struct k_timer *dummy) {
	SYSTEM_TIME_SECONDS++;
	if (SYSTEM_TIME_SECONDS >= 24 * 60 * 60) {
		// 1 full day has passed, reset the counter
		SYSTEM_TIME_SECONDS -= 24 * 60 * 60;
	}
	return;
}

void configure_timers(void) {	
	k_timer_init(&display_timeout, display_timeout_isr, NULL );
	k_timer_start(&display_timeout, K_SECONDS(5), K_SECONDS(5));

	SYSTEM_TIME_SECONDS = 0;
	k_timer_init(&clock_increment_timer, time_increment_isr, NULL);
	k_timer_start(&clock_increment_timer, K_SECONDS(1), K_SECONDS(1));
}

void SYSTEM_init(void) {

	show_time = true;
	show_percent = false;
	show_voltage = false;

	ADC_init();
	Display_init();
	IMU_init();
	configure_timers();
	RTC_init();
	HR_init();
}

int thread_main_DEV(void) {
	// TODO:
	// Test all the functions that I wrote for the RTC
	// Write a processing algorithm that measures heart rate
	// Write the handling funcitons that allow for saving to flash
	// This should be used as a development thread.
	SYSTEM_init();
	// HR_disable();
	RTC_set_time(1, 5, 0);

	while(1) {
		if (RTC_get_time() % 2) {
			illuminate_green(10);
			// turn_off_red();
		} else {
			// illuminate_red(10);
			turn_off_green();
		}
		k_msleep(100);
	}
}

int thread_main(void) {

	extern bool BLE_RECIEVED_FLAG;

	SYSTEM_init();

	// while(1) {
	// 	display_arb(2,1);
	// 	k_msleep(1000);
	// 	display_arb(1,1);
	// 	k_msleep(1000);
	// }

	while (1) { // Start here at every on-condition

		main_thread_enabled = true;

		// Enter the display loop
		while(true) {
			
			battery_mv = read_battery_voltage();
			battery_p = get_battery_percentage(battery_mv);
			if ((battery_mv < BATTERY_MIN_VOLTAGE_MV)){
					// Make sure that we do nothing if the battery is too low.
					break;
			}

			if (show_time) {
				display_time_seconds_mil(SYSTEM_TIME_SECONDS, military_time);
			} else if (show_percent) {
				display_percent(battery_p);
			} else if (show_voltage) {
				display_battery_voltage_mv(battery_mv);
			} else {
				break; // If this statement is reached, this means that the display timeout has executed.
			}
		}
		BLE_RECIEVED_FLAG = false;
		disableSegments();
		main_thread_enabled = false;
		k_thread_suspend(thread_main_id);
	}
	return 0;
}

void resume_main_thread(void) {
	k_thread_resume(thread_main_id);
	if (!main_thread_enabled) {
		k_timer_init(&display_timeout, display_timeout_isr, NULL);
		k_timer_start(&display_timeout, K_SECONDS(5), K_SECONDS(5));
	}
}

void continue_showing_time(void){
	show_time = true;
	show_percent = false;
	show_voltage = false;
	
	resume_main_thread();
}

void continue_showing_battery_percent(void){
	show_time = false;
	show_percent = true;
	show_voltage = false;

	resume_main_thread();
}

void continue_showing_battery_voltage(void){
	show_time = false;
	show_percent = false;
	show_voltage = true;

	resume_main_thread();
}

void set_time(uint32_t time) {
	SYSTEM_TIME_SECONDS = time;

	continue_showing_time();
}

void set_military_time(bool status) {
	// true = enabled, false = disabled
	military_time = status;

	continue_showing_time();
}

void simulate_IMU_interrupt(void) {
	continue_showing_time();
}

void set_always_on(bool state) {
	always_on = state;
	if (always_on) {
		simulate_IMU_interrupt();
	}
}

void set_always_on_while_charging(bool state) {
	always_on_while_charging = state;
	if (is_charging) {
		simulate_IMU_interrupt();
	}
}