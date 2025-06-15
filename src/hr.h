#ifndef HR_H
#define HR_H

#include "main.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#define RED_CHANNEL 1
#define GREEN_CHANNEL 2
#define BLUE_CHANNEL 3
#define IR_CHANNEL 4

static uint8_t RED_CURRENT_mA = 5; 
static uint8_t GREEN_CURRENT_mA = 3;
static uint8_t BLUE_CURRENT_mA = 5; 
static uint8_t IR_CURRENT_mA = 5;

#define I2C0_LED_DRV_NODE DT_NODELABEL(is31fl3294)
static const struct i2c_dt_spec led_drv_i2c = I2C_DT_SPEC_GET(I2C0_LED_DRV_NODE);

#define I2C0_ALS_NODE DT_NODELABEL(apds_9253_001)
static const struct i2c_dt_spec als_i2c = I2C_DT_SPEC_GET(I2C0_ALS_NODE);

static uint32_t ALS_IR_READING;
static uint32_t ALS_BLUE_READING;
static uint32_t ALS_GREEN_READING;
static uint32_t ALS_RED_READING;

void HR_init(void);
void HR_LED_driver_init(void);
void HR_ALS_init(void);

void HR_enable(void);
void HR_disable(void);

void HR_ALS_collect_sample(void);
uint32_t HR_ALS_get_recorded_red_value(void);
uint32_t HR_ALS_get_recorded_green_value(void);
uint32_t HR_ALS_get_recorded_blue_value(void);
uint32_t HR_ALS_get_recorded_IR_value(void);

uint8_t write_to_HR_LED_driver(uint8_t reg, uint8_t val);
uint8_t read_from_HR_LED_driver(uint8_t reg);

uint8_t write_to_HR_ALS(uint8_t reg, uint8_t val);
uint8_t read_from_HR_ALS(uint8_t reg);

uint32_t HR_collect_red_sample(void);
uint32_t HR_collect_IR_sample(void);
uint32_t HR_collect_green_sample(void);

void HR_LED_illuminate_channel(uint8_t channel_number, uint8_t current_mA);
void HR_LED_turn_off_channel(uint8_t channel_number);

void HR_LED_illuminate_red(uint8_t current_mA);
void HR_LED_turn_off_red(void);

void HR_LED_illuminate_green(uint8_t current_mA);
void HR_LED_turn_off_green(void);

void HR_LED_illuminate_blue(uint8_t current_mA);
void HR_LED_turn_off_blue(void);

void HR_LED_illuminate_IR(uint8_t current_mA);
void HR_LED_turn_off_IR(void);

#endif