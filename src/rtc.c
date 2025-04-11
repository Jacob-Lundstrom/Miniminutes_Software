#include "rtc.h"

uint8_t write_to_RTC(uint8_t reg, uint8_t val) {

	while (!device_is_ready(rtc_i2c.bus));
	uint8_t config[2] = {reg, val};
	int ret = i2c_write_dt(&rtc_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

uint8_t read_from_RTC(uint8_t reg) {

	while (!device_is_ready(rtc_i2c.bus));
	uint8_t config[1] = {reg};
	int ret = i2c_write_dt(&rtc_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	uint8_t data;
	ret = i2c_read_dt(&rtc_i2c, &data, sizeof(data));
	if(ret != 0){
		return 1;
	}

	return data;
}

void RTC_init(void) {
    write_to_RTC(0x00, 0b01011000); // Initiates a software reset
}

void RTC_set_time(uint8_t hour, uint8_t minute, uint8_t second) {
    RTC_set_time_seconds(second);
    RTC_set_time_minutes(minute);
    RTC_set_time_hour(hour);
}

void RTC_set_date(uint8_t year, uint8_t month, uint8_t day){
    RTC_set_time_day(day);
    RTC_set_time_month(month);
    RTC_set_time_year(year);
}

uint8_t encode(uint8_t input) {
    uint8_t b7 = input / 80;
    uint8_t b6 = (input - b7 * 80) / 40;
    uint8_t b5 = (input - b7 * 80 - b6 * 40) / 20;
    uint8_t b4 = (input - b7 * 80 - b6 * 40 - b5 * 20) / 10;
    uint8_t b3 = (input - b7 * 80 - b6 * 40 - b5 * 20 - b4 * 10) / 8;
    uint8_t b2 = (input - b7 * 80 - b6 * 40 - b5 * 20 - b4 * 10 - b3 * 8) / 4;
    uint8_t b1 = (input - b7 * 80 - b6 * 40 - b5 * 20 - b4 * 10 - b3 * 8 - b2 * 4) / 2;
    uint8_t b0 = (input - b7 * 80 - b6 * 40 - b5 * 20 - b4 * 10 - b3 * 8 - b2 * 4 - b1 * 2) / 1;
    return (b7 << 7) + (b6 << 6) + (b5 << 5) + (b4 << 4) + (b3 << 3) + (b2 << 2 ) + (b1 << 1) + (b0); 
}

uint8_t decode(uint8_t input) {
    return ((input >> 7) * 80) + ((input >> 6 & 1) * 40) + ((input >> 5 & 1) * 20) + ((input >> 4 & 1) * 10) +
    ((input >> 3 & 1) * 8) + ((input >> 2 & 1) * 4) + ((input >> 1 & 1) * 2) + (input & 1);
}

void RTC_set_time_seconds(uint8_t second) {
    if (second > 59) return;
    write_to_RTC(0x04, encode(second));
}
void RTC_set_time_minutes(uint8_t minute) {
    if (minute > 59) return;
    write_to_RTC(0x05, encode(minute));
}
void RTC_set_time_hour(uint8_t hour) {
    if (hour > 24) return;
    write_to_RTC(0x06, encode(hour));
}
void RTC_set_time_day(uint8_t day) {
    if (day > 31) return;
    write_to_RTC(0x07, encode(day));
}
void RTC_set_time_month(uint8_t month) {
    if (month > 12) return;
    write_to_RTC(0x09, encode(month));
}
void RTC_set_time_year(uint8_t year){
    if (year > 99) return;
    write_to_RTC(0x0A, encode(year));
}

void RTC_set_alarm_time(uint8_t year, int month, int day, int hour, int minute, int second) {
    RTC_set_alarm_time_day(day);
    RTC_set_alarm_time_hour(hour);
    RTC_set_alarm_time_minute(minute);
    RTC_set_alarm_time_second(second);
}
void RTC_set_alarm_time_second(uint8_t second){
    if (second > 59) return;
    write_to_RTC(0x0B, encode(second));
}
void RTC_set_alarm_time_minute(uint8_t minute){
    if (minute > 59) return; 
    write_to_RTC(0x0C, encode(minute));
}
void RTC_set_alarm_time_hour(uint8_t hour){
    if (hour > 24) return; 
    write_to_RTC(0x0D, encode(hour));
}
void RTC_set_alarm_time_day(uint8_t day){
    if (day > 31) return;
    write_to_RTC(0x0E, encode(day));
}

void RTC_disable_alarm(void){
    write_to_RTC(0x01, 0x00);
}

uint32_t RTC_get_time(void){
    uint32_t s = RTC_get_time_seconds();
    uint32_t m = RTC_get_time_minutes();
    uint32_t h = RTC_get_time_hour();

    return (s + 60 * m + 3600 * h);
}

uint32_t RTC_get_date(void){

}

uint8_t RTC_get_time_seconds(void) {
    return decode(read_from_RTC(0x04));
}

uint8_t RTC_get_time_minutes(void) {
    return decode(read_from_RTC(0x05));
}

uint8_t RTC_get_time_hour(void) {
    return decode(read_from_RTC(0x06));
}

uint8_t RTC_get_time_day(void) {
    return decode(read_from_RTC(0x07));
}

uint8_t RTC_get_time_month(void) {
    return decode(read_from_RTC(0x09));
}

uint8_t RTC_get_time_year(void) {
    return decode(read_from_RTC(0x0A));
}


uint8_t RTC_check_alarm(void) {
    bool alarm = (read_from_RTC(0x01));
    if (alarm &  0b01000000) write_to_RTC(0x01, alarm & 0b10111111); // Clear the flag
    return (alarm & 0b01000000) >> 6;
}