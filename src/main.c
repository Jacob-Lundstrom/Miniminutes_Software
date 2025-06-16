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
#include "display_als.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(smp_sample);

#define ALARM_CHANNEL_ID 0
#define TIMER DT_NODELABEL(rtc0)


#define MOTOR_NODE DT_ALIAS(motorenable)
static const struct gpio_dt_spec motor = GPIO_DT_SPEC_GET(MOTOR_NODE, gpios);


static struct k_timer display_timeout;
static struct k_timer clock_increment_timer;

const k_tid_t thread_main_id;
const k_tid_t display_thread_id;
const k_tid_t battery_monitor_thread_id;
#define MAIN_STACKSIZE 512*16
#define PRIORITY 7

static bool military_time = false;

K_THREAD_DEFINE(thread_main_id, MAIN_STACKSIZE, THREAD_main, NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(ble_thread_id, 2 * BLE_STACKSIZE, BLE_init, NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(battery_monitor_thread_id, 512, THREAD_battery_monitor, NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(display_thread_id, 512, THREAD_display, NULL, NULL, NULL,
	PRIORITY, 0, 0);

uint16_t RDG_DISPLAY_DEV = 0;
char DISPLAY_MESSAGE[5] = {'?','?','?','?','?'};

int THREAD_battery_monitor (void) {
	while(1) {
		k_msleep(9900);
		if (is_charging) {
			PWR_disconnect_from_charger();
			k_msleep(100); // Wait for the voltage on the cap to stabilize
			battery_mv = read_battery_voltage(); // With battery tracking mode, the voltage on SYS is about 0.225 V higher than the battery.
			PWR_reconnect_to_charger();
		} else {
			k_msleep(100);
			battery_mv = read_battery_voltage();
		}
		battery_p = get_battery_percentage(battery_mv);
	}
}


int THREAD_display (void) {
	extern bool BLE_RECIEVED_FLAG;

	while(true) {
		if ((battery_mv < BATTERY_MIN_VOLTAGE_MV)){
				// Make sure that we do nothing if the battery is too low.
				k_thread_suspend(display_thread_id);
		} else {
			if (show_time) {
				uint32_t t = RTC_get_time();
				Display_display_time_seconds(t, military_time, t % 2 );
			} else if (show_percent) {
				Display_display_percent(battery_p);
			} else if (show_voltage) {
				Display_display_battery_voltage_mv(battery_mv);
			} else if (show_message) {
				Display_display_word(DISPLAY_MESSAGE, sizeof(DISPLAY_MESSAGE), show_message_green, show_message_red);
			} else {
				for (int i = 0; i<sizeof(DISPLAY_MESSAGE); i++) DISPLAY_MESSAGE[i] = '?';
				k_thread_suspend(display_thread_id);
			}
		}
	}
}

int THREAD_display_DEV (void) {
	while (1)
		Display_display_word(DISPLAY_MESSAGE, sizeof(DISPLAY_MESSAGE),true, true);
}

void stop_display(void) {
	need_to_check_input = false;
	show_time = false;
	show_percent = false;
	show_voltage = false;
	show_message = false;

	display_timeout_isr(NULL);
}

void continue_check_input(void) {
	need_to_check_input = true;
	
	show_time = false;
	show_percent = false;
	show_voltage = false;
	show_message = false;
	
	resume_main_thread();
}

void IMU_wakeup_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
	if (show_time || show_percent || show_voltage || show_message){
		return;
	} else {
		continue_showing_time();
	}
}

void PWR_wakeup_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
	continue_check_input();
}

void display_timeout_isr(struct k_timer *dummy) {
	if (!(always_on || (charging_display_mode && is_charging))) {
		show_time = false;
		show_percent = false;
		show_voltage = false;
		show_message = false;
	}
	k_timer_stop(&display_timeout);
}

void configure_timers(void) {	
	k_timer_init(&display_timeout, display_timeout_isr, NULL );
	k_timer_start(&display_timeout, K_SECONDS(5), K_SECONDS(5));
}

void display_while_charging(void) {
	if (is_charging) {
		if (charging_display_mode == DISPLAY_NOTHING_WHILE_CHARGING) stop_display();
		if (charging_display_mode == DISPLAY_PERCENT_WHILE_CHARGING) continue_showing_battery_percent();
		if (charging_display_mode == DISPLAY_VOLTAGE_WHILE_CHARGING) continue_showing_battery_voltage();
		if (charging_display_mode == DISPLAY_TIME_WHILE_CHARGING) continue_showing_time();
		if (charging_display_mode == DISPLAY_TIME_AND_PERCENT_WHILE_CHARGING) return; // unimplemented at the moment
	}
}

void SYSTEM_init(void) {

	need_to_check_input = true; // On startup, see if the device is on the charger

	// Assume that it is not charging
	show_time = false;
	show_percent = false;
	show_voltage = false;

	Display_ALS_init(); // Only for MicroMinutes
	Display_init(); // Only for MicroMinutes
	Motor_init(); // Only for MicroMinutes

	// HR_init(); // Only for MicroFitness
	
	PWR_init();
	ADC_init();
	// IMU_init();

	// RTC reset should never be needed. This will reset the clock and lose the current time.
	// RTC_reset();
	// RTC_enable_half_minute_interrupt();
	// RTC_enable_minute_interrupt();
	// RTC_enable_timer_interrupts();

}

#include "display.h"
int THREAD_main_DEV(void) {

	// SYSTEM_init();

	// while(1) {
	// 	k_msleep(1000);
	// 	uint16_t b = Display_ALS_get_brightness();
	// 	// if (b > 0)
	// 		RDG_DISPLAY_DEV = b;
	// }

	// TODO:
	// Test all the functions that I wrote for the RTC
	// Write a processing algorithm that measures heart rate
	// Write the handling funcitons that allow for saving to flash
	// This should be used as a development thread.
	SYSTEM_init();
	// HR_disable();
	RTC_set_time(0, 0, 0);
	continue_showing_time();

	while(1) {
		k_msleep(100);
	}

	int64_t red_avg = 0;
	int64_t green_avg = 0;

	while(1) {
		int N = 200;
		uint16_t red_rdg[N];
		uint16_t ir_rdg[N];
		uint16_t green_rdg[N];
		// for (int i = 0; i < N; i++) {
		// 	red_rdg[i] = HR_collect_red_sample();
		// }
		for (int i = 0; i < N; i++) {
			green_rdg[i] = HR_collect_green_sample();
		}
		// for (int i = 0; i < N; i++) {
		// 	red_rdg[i] = HR_collect_red_sample();
		// 	ir_rdg[i] = HR_collect_IR_sample();
		// }
		k_msleep(1000);
	}
}

int THREAD_main(void) {
	// TODO
	// - Check all modes for full functionality
	// - Implement OTA updates
	// - Add Step counting
	// - Add timer setting functionality (alarms)
	// - Suspension of BLE thread when powered down
	// - Further improvements of power consumptino when powered down
	// - Perform burn-out tests on eahc color of LED
	// - Finish next board revision, place order
	// 		- Add vibration functionality, fix IMU problem

	SYSTEM_init();		

	// This is required for the display to begin showing info immediately after startup
	k_msleep(1000); // wait for the other threads to finish initializing before starting

	is_charging = PWR_get_is_on_charger();
	// set_display_mode_while_charging(DISPLAY_PERCENT_WHILE_CHARGING);
	set_display_mode_while_charging(DISPLAY_VOLTAGE_WHILE_CHARGING);
	need_to_check_input = true;

	while(1) {
		if (need_to_check_input) {
			// is_charging = PWR_get_is_on_charger();
			// bool rtc_alarm_flag = RTC_check_alarm_flag();
			// bool rtc_timer_flag = RTC_check_timer_flag();
			is_charging = 0;
			bool rtc_alarm_flag = false;
			bool rtc_timer_flag = false;


			need_to_check_input = false;
			if (rtc_alarm_flag) {
				continue_showing_time();
			} else if (rtc_timer_flag) {
				continue_showing_time();
			}
			if (is_charging) display_while_charging();
			
			k_msleep(100);
		} else {
			k_thread_suspend(thread_main_id);
		}
	}

	return 0;
}

void resume_display(void) {
	k_timer_stop(&display_timeout);
	k_timer_init(&display_timeout, display_timeout_isr, NULL);
	k_timer_start(&display_timeout, K_SECONDS(5), K_SECONDS(5));
	k_thread_resume(display_thread_id);
}

void resume_main_thread(void) {
	k_timer_stop(&display_timeout);
	k_timer_init(&display_timeout, display_timeout_isr, NULL);
	k_timer_start(&display_timeout, K_SECONDS(5), K_SECONDS(5));
	k_thread_resume(thread_main_id);
}

void continue_showing_time(void){
	show_time = true;
	show_percent = false;
	show_voltage = false;
	
	show_message = false;

	resume_display();
}

void continue_showing_battery_percent(void){
	show_time = false;
	show_percent = true;
	show_voltage = false;

	show_message = false;

	resume_display();
}

void continue_showing_battery_voltage(void){
	show_time = false;
	show_percent = false;
	show_voltage = true;

	show_message = false;

	resume_display();
}

void continue_showing_message(char *msg, uint8_t length, bool green_status, bool red_status){
	show_time = false;
	show_percent = false;
	show_voltage = false;

	show_message = true;
	show_message_green = green_status;
	show_message_red = red_status;
	for (int i = 0; i < sizeof(DISPLAY_MESSAGE); i++) DISPLAY_MESSAGE[i] = '?';
	for (int i = 0; i < sizeof(DISPLAY_MESSAGE) & i < length; i++) DISPLAY_MESSAGE[i] = msg[i];
	
	resume_display();
}

void set_time(uint32_t time) {
	uint8_t h = time / 3600; // Number of hours
	uint8_t m = (time - 3600 * h) / 60; // Number of minutes
	uint8_t s = (time - 3600 * h - 60 * m); 
	RTC_set_time(h, m, s);

	continue_showing_time();
}

void set_military_time(bool status) {
	// true = enabled, false = disabled
	military_time = status;

	continue_showing_time();
}

void simulate_IMU_interrupt(void) {
	continue_showing_time();
	// PWR_wakeup_isr(NULL, NULL, 0);
}

void set_always_on(bool state) {
	// This function should be used mostly for development only.
	always_on = state;
	if (always_on) {
		simulate_IMU_interrupt();
	} else {
		stop_display();
	}
}

void set_display_mode_while_charging(uint8_t state) {
	charging_display_mode = state;
	if (is_charging) {
		display_while_charging();
	}
}


void Motor_init(void) {
	if (!gpio_is_ready_dt(&motor)) {
		return 0;
	}

	int ret = gpio_pin_configure_dt(&motor, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		return 0;
	}

	Motor_off();
}

void Motor_on(void) {
	gpio_pin_set_dt(&motor, 1);
	// gpio_pin_configure_dt(&motor, GPIO_OUTPUT_ACTIVE);
}

void Motor_off(void) {
	gpio_pin_set_dt(&motor, 0);
	// gpio_pin_configure_dt(&motor, GPIO_INPUT);
}