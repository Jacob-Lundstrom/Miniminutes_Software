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

#include "main.h"
#include "display.h"
#include "imu.h"
#include "bluetooth.h"
#include "pwr.h"

#define ALARM_CHANNEL_ID 0
#define TIMER DT_NODELABEL(rtc0)

static struct k_timer display_timeout;
static struct k_timer clock_increment_timer;

const k_tid_t thread_main_id;
#define MAIN_STACKSIZE 512
#define PRIORITY 7

static bool military_time = false;

#define TIMER_DEVICE_NODE DT_NODELABEL(timer0)

K_THREAD_DEFINE(thread_main_id, MAIN_STACKSIZE, thread_main, NULL, NULL, NULL,
		PRIORITY, 0, 0);

// K_THREAD_STACK_DEFINE(ble_thread_stack, BLE_STACKSIZE);
// static k_tid_t ble_thread_id;

K_THREAD_DEFINE(ble_thread_id, BLE_STACKSIZE, BLE_init, NULL, NULL, NULL,
		PRIORITY, 0, 0);


void display_timeout_isr(struct k_timer *dummy) {
	show_time = false;
	show_percent = false;
	show_voltage = false;
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

void IMU_wakeup_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
	if (show_time || show_percent || show_voltage){
		return;
	} else {
		continue_showing_time();
	}
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

	Display_init();
	IMU_init();
	configure_timers();
	ADC_init();
}

int thread_main(void) {
	extern bool BLE_RECIEVED_FLAG;

	SYSTEM_init();
	
	while (1) { // Start here at every on-condition

		// Measure the battery before entering the display loop
		battery_mv = read_battery_voltage();
		battery_p = get_battery_percentage(battery_mv);

		// Enter the display loop
		if (battery_mv < BATTERY_MIN_VOLTAGE_MV) {
				// Make sure that we do nothing if the battery is too low.
		} else {
			while(true) {
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
		}
		disableSegments();
		k_thread_suspend(thread_main_id);
	}
	return 0;
}

void resume_main_thread(void) {
	k_thread_resume(thread_main_id);
	k_timer_init(&display_timeout, display_timeout_isr, NULL );
	k_timer_start(&display_timeout, K_SECONDS(5), K_SECONDS(5));
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

void reset_BLE(void) {
	// k_thread_abort(ble_thread_id);
	// start_BLE_thread();
}

void start_BLE_thread(void){
    // k_thread_create(
    //     &ble_thread_id,
	// 	ble_thread_stack,
	// 	K_THREAD_STACK_SIZEOF(ble_thread_stack),
    //     BLE_init,
    //     NULL, NULL, NULL,
    //     PRIORITY, 0, K_NO_WAIT);
}