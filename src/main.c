/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

// THIS PROGRAM REQUIRES nRF Connect SDK v2.6.1
// I don't think that the toolchain version matters.

#include "main.h"


const k_tid_t thread_main_id;
const k_tid_t display_thread_id;
const k_tid_t battery_monitor_thread_id;

K_THREAD_DEFINE(thread_main_id, MAIN_STACKSIZE, THREAD_main, NULL, NULL, NULL,
		PRIORITY, 0, 0);

// K_THREAD_DEFINE(ble_thread_id, 2 * BLE_STACKSIZE, BLE_init, NULL, NULL, NULL,
// 		PRIORITY, 0, 0);

K_THREAD_DEFINE(battery_monitor_thread_id, 512, THREAD_battery_monitor, NULL, NULL, NULL,
		PRIORITY, 0, 0);

K_THREAD_DEFINE(display_thread_id, 512, THREAD_display, NULL, NULL, NULL,
	PRIORITY, 0, 0);

	
int THREAD_battery_monitor (void) {
	while(1) {
		k_msleep(500);
		if (display_thread_id->base.thread_state & _THREAD_SUSPENDED) {
			k_msleep(9500);
		}

		battery_mv = read_battery_voltage(); // With battery tracking mode, the voltage on SYS is about 0.225 V higher than the battery.
		if (PWR_get_is_on_charger()) battery_mv -= 225;

		battery_p = get_battery_percentage(battery_mv);
		continue_check_common_interrupt(); // This makes sure that we check that no interrupts occurred while measuring the battery
	}
}

int THREAD_display (void) {
	while(true) {
		if ((battery_mv < BATTERY_MIN_VOLTAGE_MV)){
				// Make sure that we do nothing if the battery is too low.
				k_thread_suspend(display_thread_id);
		} else {
			if (show_time) {
				uint32_t t = RTC_get_time();
				// Display_display_time_seconds(t, military_time, t % 2 );
			} else if (show_percent) {
				Display_display_percent(battery_p);
			} else if (show_voltage) {
				Display_display_battery_voltage_mv(battery_mv);
			} else if (show_message) {
				Display_display_word(DISPLAY_MESSAGE, sizeof(DISPLAY_MESSAGE), show_message_green, show_message_red);
			} else {
				for (int i = 0; i<sizeof(DISPLAY_MESSAGE); i++) DISPLAY_MESSAGE[i] = '?';
				Display_ALS_disable();
				Display_sleep();
				k_thread_suspend(display_thread_id);
				// the display will always start at this line on every resume.
				Display_ALS_enable();
			}
		}
	}
}

int THREAD_display_DEV (void) {
	while (1)
		// Display_display_word(DISPLAY_MESSAGE, sizeof(DISPLAY_MESSAGE),true, true);
		Display_display_integer(RDG_DISPLAY_DEV, false, false);
}

int THREAD_main_DEV(void) {

	SYSTEM_init();

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

	SYSTEM_init();		

	// This is required for the display to begin showing info immediately after startup
	k_msleep(1000); // wait for the other threads to finish initializing before starting

	is_charging = PWR_get_is_on_charger();
	set_display_mode_while_charging(DISPLAY_VOLTAGE_WHILE_CHARGING);
	set_display_auto_brightness(true);
	need_to_check_interrupt_source = true;

	// Do the same thing that the battery monitor thread would do.
	battery_mv = read_battery_voltage();
	
	k_thread_resume(display_thread_id); // Properly start the display thread once the ADC recorded the "battery" voltage
	// This is required to ensure that the ALS will start up on the first display event.

	while(1) {
		if (need_to_check_interrupt_source) {
			is_charging 			= PWR_get_is_on_charger();
			bool rtc_alarm_flag 	= RTC_check_alarm_flag();
			bool rtc_timer_flag 	= RTC_check_timer_flag();
			bool disp_als_int_flag 	= Display_ALS_check_interrupts();
			need_to_check_interrupt_source 	= false; // At this point we should know which source the interrupt came from

			if (disp_als_int_flag) {
				static double b;
				if (display_auto_brightness) {
					b = (double) Display_ALS_get_brightness() / (1000.0);
					if (b > 0) Display_set_duty_cycle(b);
				}
				Display_ALS_clear_interrupts();
				disp_als_int_flag = false;
			}
			Display_ALS_clear_interrupts(); // There's a glitch sometimes where the ALS gets stuck low... This fixes it?

			if (rtc_alarm_flag) {
				continue_showing_time();
			}

			if (rtc_timer_flag) {
				continue_showing_time();
			}

			if (is_charging) display_while_charging();
			
		} 
		
		if (need_to_check_IMU_interrupt) {
			uint8_t int_src = read_from_IMU(0x24);
			if (int_src) { // If the wakeup source was double tap, show time.
				continue_showing_time();
			}
			need_to_check_IMU_interrupt = false;			
		} 
		
		k_thread_suspend(thread_main_id);
	}

	return 0;
}

void stop_display(void) {
	need_to_check_interrupt_source = false;
	show_time = false;
	show_percent = false;
	show_voltage = false;
	show_message = false;

	display_timeout_isr(NULL);
}

void IMU_interrupt_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
	need_to_check_IMU_interrupt = true;
	resume_main_thread();
}

void common_interrupt_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
	continue_check_common_interrupt();
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

	need_to_check_interrupt_source = true; // On startup, see if the device is on the charger

	// Assume that it is not charging
	show_time = false;
	show_percent = false;
	show_voltage = false;

	// MicroMinutes inits
	Display_init();
	Motor_init();

	// MicroFitness inits
	// HR_init();

	// Always initialized
	PWR_init();
	ADC_init();
	IMU_init();

	// RTC reset should never be needed. This will reset the clock and lose the current time.
	// Since we want to retain the current time through a UVLO, Don't reset on start up.
	// RTC_reset();
}

void resume_display(void) {
	k_timer_stop(&display_timeout);
	k_timer_init(&display_timeout, display_timeout_isr, NULL);
	k_timer_start(&display_timeout, K_SECONDS(5), K_SECONDS(5));
	k_thread_resume(display_thread_id);
}

void resume_main_thread(void) {
	k_thread_resume(thread_main_id);
}

void continue_check_common_interrupt(void) {
	need_to_check_interrupt_source = true;
	resume_main_thread();
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

void set_display_auto_brightness(bool state) {
	display_auto_brightness = state;
}

void simulate_IMU_interrupt(void) {
	continue_showing_time();
}

// MOTOR FUNCTIONS

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

void Motor_vibrate(uint32_t duration_us) {
	Motor_on();
	k_timer_init(&motor_timeout, Motor_timeout_isr, NULL );
	k_timer_start(&motor_timeout, K_USEC(duration_us), K_USEC(duration_us));
}

void Motor_timeout_isr(struct k_timer *dummy) {
	Motor_off();
	k_timer_stop(&motor_timeout);
}