#ifndef DISPLAY_H
#define DISPLAY_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include "pwr.h"

#define CA1_NODE DT_ALIAS(ca1)
#define CA2_NODE DT_ALIAS(ca2)
#define CA3_NODE DT_ALIAS(ca3)
#define CA4_NODE DT_ALIAS(ca4)
#define CA5_NODE DT_ALIAS(ca5)

#define CC1_NODE DT_ALIAS(cc1)
#define CC2_NODE DT_ALIAS(cc2)
#define CC3_NODE DT_ALIAS(cc3)
#define CC4_NODE DT_ALIAS(cc4)
#define CC5_NODE DT_ALIAS(cc5)
#define CC6_NODE DT_ALIAS(cc6)
#define CC7_NODE DT_ALIAS(cc7)

#define SEGMENT_MIN_ON_TIME_US 100
#define SEGMENT_MIN_OFF_TIME_US 100

static const struct gpio_dt_spec CA1 = GPIO_DT_SPEC_GET(CA1_NODE, gpios);
static const struct gpio_dt_spec CA2 = GPIO_DT_SPEC_GET(CA2_NODE, gpios);
static const struct gpio_dt_spec CA3 = GPIO_DT_SPEC_GET(CA3_NODE, gpios);
static const struct gpio_dt_spec CA4 = GPIO_DT_SPEC_GET(CA4_NODE, gpios);
static const struct gpio_dt_spec CA5 = GPIO_DT_SPEC_GET(CA5_NODE, gpios);

static const struct gpio_dt_spec CC1 = GPIO_DT_SPEC_GET(CC1_NODE, gpios);
static const struct gpio_dt_spec CC2 = GPIO_DT_SPEC_GET(CC2_NODE, gpios);
static const struct gpio_dt_spec CC3 = GPIO_DT_SPEC_GET(CC3_NODE, gpios);
static const struct gpio_dt_spec CC4 = GPIO_DT_SPEC_GET(CC4_NODE, gpios);
static const struct gpio_dt_spec CC5 = GPIO_DT_SPEC_GET(CC5_NODE, gpios);
static const struct gpio_dt_spec CC6 = GPIO_DT_SPEC_GET(CC6_NODE, gpios);
static const struct gpio_dt_spec CC7 = GPIO_DT_SPEC_GET(CC7_NODE, gpios);


int Display_init();
void enableSegment(int segment);
void disableSegment(int segment);
void enableSegmentLow(int segment);
void disableSegments();
void displayNone();
void displayNumberOnSegment(uint8_t num, int segment);
void displayColon();
void displayTime(int num1, int num2, bool colon, int num3, int num4, bool green_ind, bool red_ind);
void display_time_seconds(int current_time_seconds);
void display_time_seconds_mil(int current_time_seconds, bool military_time);
void display_battery_voltage_mv(int mv);
void display_error(uint8_t error_number);
void display_credits(void);
void display_percent(int percent);

// HELPER FUNCTIONS

// This is a helper function that converts a given number to the corresponding number 
// that should be used to display that number on a 7-segment display.
uint8_t num_to_segment(uint8_t number);

// This is a helper function that converts a given number to the corresponding number 
// that should be used to display that number on a 7-segment display.
uint8_t char_to_segment(uint8_t character);

// Quick display functions

void display_arb(int segment, uint8_t data);
void display_arb_all(uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5, bool green_ind, bool red_ind);
void display_word_chars(char c1, char c2, char c3, char c4, char c5, bool green_ind, bool red_ind);
void display_word(char *word, int len, bool green_ind, bool red_ind);

#endif