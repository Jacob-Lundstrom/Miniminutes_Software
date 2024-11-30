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
#include <zephyr/drivers/counter.h>
#include <zephyr/sys/printk.h>

#include "main.h"
#include "display.h"
#include "imu.h"
#include "bluetooth.h"
#include "pwr.h"

#define DELAY 10000000
#define ALARM_CHANNEL_ID 0
#define TIMER DT_NODELABEL(rtc0)
struct counter_alarm_cfg alarm_cfg;

static struct k_timer display_timeout;
static struct k_timer clock_increment_timer;

const k_tid_t thread_main_id;
#define MAIN_STACKSIZE 512
#define PRIORITY 7

static bool military_time = false;

#define TIMER_DEVICE_NODE DT_NODELABEL(timer0)


void display_timeout_isr(struct k_timer *dummy) {
	show_time = false;
	show_percent = false;
	show_voltage = false;
	k_timer_stop(&display_timeout);
}

void time_increment_isr(struct k_timer *dummy) {
	SYSTEM_TIME_SECONDS++;
	while (SYSTEM_TIME_SECONDS >= 24 * 60 * 60) {
		// 1 full day has passed, reset the counter
		SYSTEM_TIME_SECONDS -= 24 * 60 * 60;
	}
	return;
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

	SYSTEM_TIME_SECONDS += 10;
	
	int hr = SYSTEM_TIME_SECONDS / (60*60) ;
	int min = SYSTEM_TIME_SECONDS / (60) - 60 * hr;

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

	struct device *const counter_dev = DEVICE_DT_GET(TIMER);

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
	struct device *const counter_dev = DEVICE_DT_GET(TIMER);

	int err = counter_set_channel_alarm(counter_dev, ALARM_CHANNEL_ID,
					&alarm_cfg);

	if(err) { 
		return;
	}

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

	SYSTEM_TIME_SECONDS = 0;
	k_timer_init(&clock_increment_timer, time_increment_isr, NULL);
	k_timer_start(&clock_increment_timer, K_SECONDS(1), K_SECONDS(1));

	// const struct device *timer_dev = DEVICE_DT_GET(TIMER_DEVICE_NODE);

    // if (!device_is_ready(timer_dev)) {
    //     printk("Timer device not ready\n");
    //     return;
    // }

    // // Set up the timer to count at 1 MHz (1 tick = 1 µs)
    // nrf_timer_frequency_set(TIMER0, NRF_TIMER_FREQ_1MHz);

    // // Start the timer
    // counter_start(timer_dev);
}

/* Get the number of channels defined on the DTS. */
#define CHANNEL_COUNT ARRAY_SIZE(channel_cfgs)

void SYSTEM_init(void){

	// k_sem_take(&ble_init_ok, K_FOREVER); // Wait for the BLE (and therefore RTC0) to initialize

	Display_init();
	IMU_init();

	configure_timers();
	// setup_rtc();
	ADC_init();
}

int thread_main(void) {

	SYSTEM_init();


	battery_mv = read_battery_voltage();
	battery_p = 100 * (battery_mv - 3600) / (4200 - 3600);
	show_time = true;
	show_percent = false;
	show_voltage = false;

	while (1) {
		if (battery_mv < 3600) {
			disableSegments();
			k_thread_suspend(thread_main_id);
		} else if (show_time) {

			while (SYSTEM_TIME_SECONDS >= 24 * 60 * 60) {
				// 1 full day has passed, reset the counter
				SYSTEM_TIME_SECONDS -= 24 * 60 * 60;
			}
			
			int t = 1 * SYSTEM_TIME_SECONDS;

			display_time_seconds_mil(t, military_time);
		} else if (show_percent) {
			display_percent(battery_p);
		} else if (show_voltage) {
			display_battery_voltage_mv(battery_mv);
		} else {
			disableSegments();
			k_thread_suspend(thread_main_id);
		}
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
	// resume_main_thread();
}

void continue_showing_battery_percent(void){
	show_time = false;
	show_percent = true;
	show_voltage = false;
	
	battery_mv = read_battery_voltage();
	battery_p = 100 * (battery_mv - 3600) / (4200 - 3600);
	// resume_main_thread();
}

void continue_showing_battery_voltage(void){
	show_time = false;
	show_percent = false;
	show_voltage = true;
	
	battery_mv = read_battery_voltage();
	battery_p = 100 * (battery_mv - 3600) / (4200 - 3600);
	// resume_main_thread();
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

K_THREAD_DEFINE(thread_main_id, MAIN_STACKSIZE, thread_main, NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(ble_thread_id, BLE_STACKSIZE, BLE_init, NULL, NULL, NULL,
		PRIORITY, 0, 0);

int main(void) {
	
}