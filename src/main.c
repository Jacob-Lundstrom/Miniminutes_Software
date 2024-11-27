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
#include <nrfx_timer.h>

#include <nrfx_clock.h>
#include <nrfx_rtc.h>

#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>
#include <zephyr/sys/printk.h>

#include "main.h"
#include "display.h"
#include "imu.h"
#include "bluetooth.h"


#define DELAY 60000000
#define ALARM_CHANNEL_ID 0
#define TIMER DT_NODELABEL(rtc0)
struct counter_alarm_cfg alarm_cfg;
const struct device *const counter_dev = DEVICE_DT_GET(TIMER);

static struct k_timer display_timeout;

const k_tid_t thread_main_id;
#define MAIN_STACKSIZE 512
#define PRIORITY 7

static uint32_t current_time_seconds = 0;
bool show_time = false;

void display_timeout_isr(struct k_timer *dummy) {
	show_time = false;
	k_timer_stop(&display_timeout);
}

static void test_counter_interrupt_fn(const struct device *counter_dev,
				      uint8_t chan_id, uint32_t ticks,
				      void *user_data)
{
	struct counter_alarm_cfg *config = user_data;
	uint32_t now_ticks;
	uint64_t now_usec;
	int now_sec;
	int err;

	err = counter_get_value(counter_dev, &now_ticks);
	if (!counter_is_counting_up(counter_dev)) {
		now_ticks = counter_get_top_value(counter_dev) - now_ticks;
	}

	if (err) {
		return;
	}

	now_usec = counter_ticks_to_us(counter_dev, now_ticks);
	now_sec = (int)(now_usec / USEC_PER_SEC);

	current_time_seconds = now_sec;
	
	int hr = current_time_seconds / (60*60) ;
	int min = current_time_seconds / (60) - 60 * hr;

	if (min % 15 == 0) { // Displays the time automatically every 15 minutes
		show_time = true;
		k_thread_resume(thread_main_id);
		k_timer_init(&display_timeout, display_timeout_isr, NULL );
		k_timer_start(&display_timeout, K_SECONDS(5), K_SECONDS(5));
	}

	err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
					user_data);
	if (err != 0) {
		return;
	}
}

int getTime(){
	uint32_t now_ticks;
	uint64_t now_usec;
	int now_sec;

	int err = counter_get_value(counter_dev, &now_ticks);
	
	if (!counter_is_counting_up(counter_dev)) {
		now_ticks = counter_get_top_value(counter_dev) - now_ticks;
	}

	if (err) {
		return 0;
	}

	now_usec = counter_ticks_to_us(counter_dev, now_ticks);
	now_sec = (int)(now_usec / USEC_PER_SEC);

	return now_sec;
}

void setup_rtc(){
	while (!device_is_ready(counter_dev));

	counter_start(counter_dev);

	alarm_cfg.flags = 0;
	alarm_cfg.ticks = counter_us_to_ticks(counter_dev, DELAY);
	alarm_cfg.callback = test_counter_interrupt_fn;
	alarm_cfg.user_data = &alarm_cfg;

	int err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
					&alarm_cfg);

	if(err) { 
		return;
	}

    /* Initialize and start the LFRC clock */
    nrfx_clock_init(NULL);
    // nrfx_clock_lfclk_start();

	while (NRF_CLOCK->EVENTS_DONE == 0);

	return;
}

void IMU_wakeup_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	show_time = true;
	k_timer_init(&display_timeout, display_timeout_isr, NULL );
	k_timer_start(&display_timeout, K_SECONDS(5), K_SECONDS(5));
	k_thread_resume(thread_main_id);
}

void configure_timers(){	
	k_timer_init(&display_timeout, display_timeout_isr, NULL );
	k_timer_start(&display_timeout, K_SECONDS(5), K_SECONDS(5));
}

void SYSTEM_init(void){

	Display_init();
	IMU_init();

	configure_timers();
	// setup_rtc();
}

int thread_main(void) {

	SYSTEM_init();

	int start_time = (0 * 60 + 3) * 60;

	current_time_seconds = start_time;

	show_time = true;
	while (1) {
		if (show_time) {
			current_time_seconds = start_time + getTime();

			if (current_time_seconds >= 24 * 60 * 60) {
				// 1 full day has passed, reset the counter
				current_time_seconds -= 24 * 60 * 60;
				// NRF_RTC0->TASKS_CLEAR = 1;
				start_time = 0;
			}

			// n = n*60; // Uncomment this line to show min:sec instead of hr:min

			int hr = current_time_seconds / (60*60) ;
			int min = current_time_seconds / (60) - 60 * hr;
			
			int dig1 = hr/10;
			int dig2 = hr - dig1 * 10;
			int dig3 = min/10;
			int dig4 = min - dig3 * 10;
			displayTime(dig1, dig2, true, dig3, dig4);
		} else {
			disableSegments();
			k_thread_suspend(thread_main_id);
		}
	}
	
	return 0;
}

K_THREAD_DEFINE(thread_main_id, MAIN_STACKSIZE, thread_main, NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(ble_thread_id, BLE_STACKSIZE, BLE_init, NULL, NULL, NULL,
		PRIORITY, 0, 0);
