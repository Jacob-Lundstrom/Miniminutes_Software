#ifndef RTC_H
#define RTC_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>


#define I2C0_RTC_NODE DT_NODELABEL(rv_8263_c8)
static const struct i2c_dt_spec rtc_i2c = I2C_DT_SPEC_GET(I2C0_RTC_NODE);

uint8_t write_to_RTC(uint8_t reg, uint8_t val);
uint8_t sequential_write_to_RTC(uint8_t reg, uint8_t *val, uint8_t length);
uint8_t read_from_RTC(uint8_t reg);
uint8_t sequential_read_from_RTC(uint8_t reg, uint8_t* data, uint8_t length);


uint8_t encode(uint8_t input);
uint8_t decode(uint8_t input);

void RTC_reset(void);
void RTC_set_time(uint8_t hour, uint8_t minute, uint8_t second);
void RTC_set_date(uint8_t year, uint8_t month, uint8_t weekday, uint8_t day);


void RTC_set_alarm_time(uint8_t weekday, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

void RTC_set_alarm_time_second(uint8_t second);
uint8_t RTC_enable_alarm_time_second(void);
void RTC_disable_alarm_time_second(void);

void RTC_set_alarm_time_minute(uint8_t minute);
uint8_t RTC_enable_alarm_time_minute(void);
void RTC_disable_alarm_time_minute(void);

void RTC_set_alarm_time_hour(uint8_t hour);
uint8_t RTC_enable_alarm_time_hour(void);
void RTC_disable_alarm_time_hour(void);

void RTC_set_alarm_time_day(uint8_t day);
uint8_t RTC_enable_alarm_time_day(void);
void RTC_disable_alarm_time_day(void);

void RTC_set_alarm_time_weekday(uint8_t weekday);
uint8_t RTC_enable_alarm_time_weekday(void);
void RTC_disable_alarm_time_weekday(void);

void RTC_enable_alarm_interrupts(void);
void RTC_disable_alarm_interrupts(void);


void RTC_enable_timer_interrupts(void);
void RTC_disable_timer_interrupts(void);

void RTC_enable_minute_interrupt(void);
void RTC_enable_half_minute_interrupt(void);

void RTC_disable_all_interrupts(void);

uint32_t RTC_get_time(void);
uint32_t RTC_get_date(void);

uint8_t RTC_check_alarm_flag(void);
uint8_t RTC_check_timer_flag(void);

#endif



