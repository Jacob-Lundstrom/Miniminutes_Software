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

#define SEGMENT_MIN_ON_TIME_US 300
#define SEGMENT_MIN_OFF_TIME_US 1

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
void Display_enable_digit(int digit);
void Display_disable_digit(int digit);
void Display_enable_digit_low(int digit);
void Display_disable_all_digits();
void Display_display_none();
void Display_display_number_on_digit(uint8_t num, int digit);
void Display_display_colon();
void Display_display_time(int num1, int num2, bool colon, int num3, int num4, bool green_ind, bool red_ind);
void Display_display_time_seconds(int current_time_seconds, bool military_time, bool show_colon);
void Display_display_battery_voltage_mv(int mv);
void Display_display_error(uint8_t error_number);
void Display_display_credits(void);
void Display_display_percent(int percent);

uint8_t Display_convert_number_to_segments(uint8_t number);
uint8_t Display_convert_character_to_segments(uint8_t character);

void Display_display_arbitrary(int digit, uint8_t data);
void Display_display_arb_all(uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5, bool green_ind, bool red_ind);
void Display_display_word_chars(char c1, char c2, char c3, char c4, char c5, bool green_ind, bool red_ind);
void Display_display_word(char *word, int len, bool green_ind, bool red_ind);
void Display_display_integer(uint16_t integer, bool green_ind, bool red_ind);

#endif