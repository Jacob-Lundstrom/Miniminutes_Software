#ifndef RTC_H
#define RTC_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>


#define I2C0_RTC_NODE DT_NODELABEL(rv_8263_c8)
static const struct i2c_dt_spec rtc_i2c = I2C_DT_SPEC_GET(I2C0_RTC_NODE);


uint8_t write_to_RTC(uint8_t reg, uint8_t val);
uint8_t read_from_RTC(uint8_t reg);

void RTC_init(void);

void RTC_set_time(uint8_t hour, uint8_t minute, uint8_t second);
void RTC_set_date(uint8_t year, uint8_t month, uint8_t day);

void RTC_set_time_seconds(uint8_t second);
void RTC_set_time_minutes(uint8_t minute);
void RTC_set_time_hour(uint8_t hour);
void RTC_set_time_day(uint8_t day);
void RTC_set_time_month(uint8_t month);
void RTC_set_time_year(uint8_t year);

void RTC_set_alarm_time(uint8_t year, int month, int day, int hour, int minute, int second);
void RTC_set_alarm_time_seconds(uint8_t second);
void RTC_set_alarm_time_minutes(uint8_t minute);
void RTC_set_alarm_time_hour(uint8_t hour);
void RTC_set_alarm_time_day(uint8_t day);
void RTC_disable_alarm(void);

uint32_t RTC_get_time(void);
uint32_t RTC_get_date(void);

uint8_t RTC_get_time_seconds(void);
uint8_t RTC_get_time_minutes(void);
uint8_t RTC_get_time_hour(void);
uint8_t RTC_get_time_day(void);
uint8_t RTC_get_time_month(void);
uint8_t RTC_get_time_year(void);

#endif