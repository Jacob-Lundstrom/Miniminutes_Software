#include "rtc.h"

/**
 * \brief 			Function to handle writing to the RTC over the I2C bus.
 * \param           reg: Register address to write to
 * \param           val: Value to write to specified register address
 * \return          0 if the I2C write was successful, 1 if there was an error in communication.
 */
uint8_t write_to_RTC(uint8_t reg, uint8_t val) {

	while (!device_is_ready(rtc_i2c.bus));
	uint8_t config[2] = {reg, val};
	int ret = i2c_write_dt(&rtc_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

/**
 * \brief 			Function to handle sequential writing to the RTC over the I2C bus.
 * \param           reg: Starting register address to write to
 * \param           val: Values to write to the device in sequential order, incrementing write register address with each byte.
 * \param           length: Total number of bytes to write to the RTC
 * \return          0 if the I2C write was successful, 1 if there was an error in communication.
 */
uint8_t sequential_write_to_RTC(uint8_t reg, uint8_t *val, uint8_t length) {

	while (!device_is_ready(rtc_i2c.bus));
	uint8_t config[length + 1];
    config[0] = reg;
    for (int i = 1; i<=length; i++) {
        config[i] = val[i - 1];
    }
	int ret = i2c_write_dt(&rtc_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

/**
 * \brief 			Function to handle reading from the RTC over the I2C bus.
 * \param           reg: Register address in the RTC to read from.
 * \return          Value stored at the requested register address.
 */
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

/**
 * \brief 			Function to handle reading from the RTC over the I2C bus.
 * \param           reg: Register address in the RTC to start sequential reads from.
 * \param           data: Buffer to store the read data in
 * \param           length: Number of bytes to read (must also be the length of the data buffer)
 * \return          0 if successful, 1 if unsuccessful.
 */
uint8_t sequential_read_from_RTC(uint8_t reg, uint8_t* data, uint8_t length) {

	while (!device_is_ready(rtc_i2c.bus));

    int ret = i2c_burst_read_dt(&rtc_i2c, reg, data, length);

	if(ret != 0){
		return 1;
	}

    return 0;
}

/**
 * \brief       Helper function that encodes some input to the corresponding BCD byte using the encoding scheme used for the RTC.
 * \param       input: Input number to encode
 * \return      BCD encoded byte
 */
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

/**
 * \brief       Helper function that decodes some BCD input using the encoding scheme used for the RTC.
 * \param       input: Input BCD number to decode
 * \return      Decoded number output
 */
uint8_t decode(uint8_t input) {
    return ((input >> 7) * 80) + ((input >> 6 & 1) * 40) + ((input >> 5 & 1) * 20) + ((input >> 4 & 1) * 10) +
    ((input >> 3 & 1) * 8) + ((input >> 2 & 1) * 4) + ((input >> 1 & 1) * 2) + (input & 1);
}

/**
 * \brief 			Initiates a software reset of the RTC. Clears all registers to their default values.
 */
void RTC_reset(void) {
    write_to_RTC(0x00, 0b01011000); // Initiates a software reset
}

/**
 * \brief 			Sets the current time kept internal to the RTC.
 * \param           hour: Hour to set the RTC to. Valid hours are 0 to 23.
 * \param           minute: Minute to set the RTC to. Valid minutes are 0 to 59.
 * \param           second: Second to set the RTC to. Valid seconds are 0 to 59.
 */
void RTC_set_time(uint8_t hour, uint8_t minute, uint8_t second) {
    if (hour > 23)      return;
    if (minute > 59)    return;
    if (second > 59)    return;
    uint8_t data[3] = {encode(second), encode(minute), encode(hour)};
    sequential_write_to_RTC(0x04, data, sizeof(data));
}

/**
 * \brief 			Sets the current date kept internal to the RTC.
 * \param           year: Year to set the RTC to. Valid years are 00-99 (2000-2099).
 * \param           month: Month to set the RTC to. Valid months are 1 to 12.
 * \param           weekday: Weekday to set the RTC to. Valid weekdays are 0 to 6.
 * \param           day: Day to set the RTC to. Valid Days are 1 to 31.
 */
void RTC_set_date(uint8_t year, uint8_t month, uint8_t weekday, uint8_t day){
    if (year > 99)                  return;
    if (weekday > 6)                return;
    if (month > 12 | month < 1)     return;
    if (day > 31 | month < 1)       return;
    uint8_t data[4] = {encode(day), encode(weekday), encode(month), encode(year)};
    sequential_write_to_RTC(0x07, data, sizeof(data));
}

/**
 * \brief       Helper function that decodes some BCD input using the encoding scheme used for the RTC.
 * \param       weekday: Weekday to set the RTC alarm to. Valid weekdays are 0 to 6 
 * \param       day: Day to set the RTC alarm to. Valid days are 1 to 31.
 * \param       hour: Hour to set the RTC to. Valid hours are 0 to 23.
 * \param       minute: Minute to set the RTC to. Valid minutes are 0 to 59.
 * \param       second: Second to set the RTC to. Valid seconds are 0 to 59.
 */
void RTC_set_alarm_time(uint8_t weekday, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    if (weekday > 6)            return;
    if (day > 31 | day < 1)     return;
    if (hour > 23)              return;
    if (minute > 59)            return;
    if (second > 59)            return;
    uint8_t data[5] = {encode(second), encode(minute), encode(hour), encode(day), encode(weekday)};
    sequential_write_to_RTC(0x07, data, sizeof(data));
}

/**
 * \brief           Sets the seconds alarm time. Alarm enable bit is unaffected.
 * \param           second: Which second within a minute for the alarm to trigger on.
 */
void RTC_set_alarm_time_second(uint8_t second){
    if (second > 59) return;
    write_to_RTC(0x0B, encode(second));
}

/**
 * \brief           Enables the seconds alarm.
 * \return          Which second within a minute the alarm will trigger on. When this coincides with all other enabled alarm timers, an interrupt will be generated.
 */
uint8_t RTC_enable_alarm_time_second(void) {
    uint8_t s = read_from_RTC(0x0B);
    write_to_RTC(0x0B, s | 0x80);
    return decode(s & 0xEF);
}

/**
 * \brief           Disables the seconds alarm. An interrupt may still be generated if other alarms are enabled.
 */
void RTC_disable_alarm_time_second(void){
    write_to_RTC(0x0B, read_from_RTC(0x0B) & 0x7F);
}

/**
 * \brief           Sets the minutes alarm time. Alarm enable bit is unaffected.
 * \param           second: Which minute within an hour for the alarm to trigger on.
 */
void RTC_set_alarm_time_minute(uint8_t minute){
    if (minute > 59) return; 
    write_to_RTC(0x0C, encode(minute));
}

/**
 * \brief           Enables the minute alarm.
 * \return          Which minute within an hour the alarm will trigger on. When this coincides with all other enabled alarms, an interrupt will be generated if alarm interrupts are enabled.
 */
uint8_t RTC_enable_alarm_time_minute(void) {
    uint8_t m = read_from_RTC(0x0C);
    write_to_RTC(0x0C, m | 0x80);
    return decode(m & 0xEF);
}

/**
 * \brief           Disables the minute alarm. An interrupt may still be generated if other alarms are enabled.
 */
void RTC_disable_alarm_time_minute(void){
    write_to_RTC(0x0C, read_from_RTC(0x0C) & 0x7F);
}

/**
 * \brief           Sets the hours alarm time. Alarm enable bit is unaffected.
 * \param           second: Which hour within a day for the alarm to trigger on.
 */
void RTC_set_alarm_time_hour(uint8_t hour){
    if (hour > 24) return; 
    write_to_RTC(0x0D, encode(hour));
}

/**
 * \brief           Enables the hour alarm.
 * \return          Which hour within a day the alarm will trigger on. When this coincides with all other enabled alarms, an interrupt will be generated if alarm interrupts are enabled.
 */
uint8_t RTC_enable_alarm_time_hour(void) {
    uint8_t h = read_from_RTC(0x0D);
    write_to_RTC(0x0D, h | 0x80);
    return decode(h & 0xEF);
}

/**
 * \brief           Disables the hour alarm. An interrupt may still be generated if other alarms are enabled.
 */
void RTC_disable_alarm_time_hour(void){
    write_to_RTC(0x0D, read_from_RTC(0x0D) & 0x7F);
}

/**
 * \brief           Sets the day alarm time. Alarm enable bit is unaffected.
 * \param           second: Which day within a month for the alarm to trigger on.
 */
void RTC_set_alarm_time_day(uint8_t day){
    if (day > 31 | day < 1) return;
    write_to_RTC(0x0E, encode(day));
}

/**
 * \brief           Enables the day alarm.
 * \return          Which day within a month the alarm will trigger on. When this coincides with all other enabled alarms, an interrupt will be generated if alarm interrupts are enabled.
 */
uint8_t RTC_enable_alarm_time_day(void) {
    uint8_t d = read_from_RTC(0x0E);
    write_to_RTC(0x0E, d | 0x80);
    return decode(d & 0xEF);
}

/**
 * \brief           Disables the day alarm. An interrupt may still be generated if other alarms are enabled.
 */
void RTC_disable_alarm_time_day(void){
    write_to_RTC(0x0E, read_from_RTC(0x0E) & 0x7F);
}

/**
 * \brief           Sets the day alarm time. Alarm enable bit is unaffected.
 * \param           second: Which day within a month for the alarm to trigger on.
 */
void RTC_set_alarm_time_weekday(uint8_t weekday){
    if (weekday > 6) return;
    write_to_RTC(0x0F, encode(weekday));
}

/**
 * \brief           Enables the weekdayday alarm.
 * \return          Which day within a week the alarm will trigger on. When this coincides with all other enabled alarms, an interrupt will be generated if alarm interrupts are enabled.
 */
uint8_t RTC_enable_alarm_time_weekday(void) {
    uint8_t wd = read_from_RTC(0x0F);
    write_to_RTC(0x0F, wd | 0x80);
    return decode(wd & 0xEF);
}

/**
 * \brief           Disables the wekday alarm. An interrupt may still be generated if other alarms are enabled.
 */
void RTC_disable_alarm_time_weekday(void){
    write_to_RTC(0x0F, read_from_RTC(0x0F) & 0x7F);
}

/**
 * \brief Configures the RTC to generate interrupts from alarms. Each Second/Minute/Hour/Day/Weekday alarm enable bits still need to be cleared for alarms to funciton.
 */
void RTC_enable_alarm_interrupts(void){
    write_to_RTC(0x01, read_from_RTC(0x01) | 0x80);
}

/**
 * \brief Configures the RTC to not generate interrupts from alarms. Minute and half minute interrupt settings are unaffected.
 */
void RTC_disable_alarm_interrupts(void){
    write_to_RTC(0x01, read_from_RTC(0x01)  & 0x7F);
}

/**
 * \brief Configures the RTC to generate interrupts from alarms. Each Second/Minute/Hour/Day/Weekday alarm enable bits still need to be cleared for alarms to funciton.
 */
void RTC_enable_timer_interrupts(void){
    write_to_RTC(0x11, read_from_RTC(0x11) | 0x02);
}

/**
 * \brief Configures the RTC to not generate interrupts from alarms. Minute and half minute interrupt settings are unaffected.
 */
void RTC_disable_timer_interrupts(void){
    write_to_RTC(0x11, read_from_RTC(0x11)  & 0xFD);
}


/**
 * \brief Configures the RTC to generate interrupts every minute. This is not effected by any alarm settings.
 */
void RTC_enable_minute_interrupt(void){
    write_to_RTC(0x01, read_from_RTC(0x01) | 0x20);
}

/**
 * \brief Configures the RTC to generate interrupts every 30 seconds. This is not effected by any alarm settings.
 */
void RTC_enable_half_minute_interrupt(void){
    write_to_RTC(0x01, read_from_RTC(0x01) | 0x10);
}

/**
 * \brief Disables all RTC interrupts. Does not modify the alarm configuration, but rather disables all output capability.
 */
void RTC_disable_all_interrupts(void) {
    write_to_RTC(0x01, 0x00);
}

/**
 * \brief           Gets the current time in seconds since midnight.
 * \return          Total count of seconds since midnight in the present day.
 */
uint32_t RTC_get_time(void){
    // Ideally, time and date should be read simultaneously. 
    // I don't have any plans to implement a date, but if I need to in the future, that is how it should be done.
    uint8_t time[3] = {0,0,0};
    sequential_read_from_RTC(0x04, time, sizeof(time));
    uint32_t s = decode(time[0] & 0x7F);
    uint32_t m = decode(time[1] & 0x7F);
    uint32_t h = decode(time[2] & 0x3F);

    return (s + 60 * m + 3600 * h);
}

/**
 * \brief           NOT YET IMPLEMENTED, DO NOT USE
 */
uint32_t RTC_get_date(void){
    uint8_t time[7] = {0,0,0,0,0,0,0};
    sequential_read_from_RTC(0x04, time, sizeof(time));
    uint32_t s = decode(time[0] & 0x7F); // Seconds
    uint32_t m = decode(time[1] & 0x7F); // Minutes
    uint32_t h = decode(time[2] & 0x3F); // Hours
    uint32_t D = decode(time[3] & 0x3F); // Day
    uint32_t W = decode(time[4] & 0x07); // Weekday Counter
    uint32_t M = decode(time[5] & 0x1F); // Month
    uint32_t Y = decode(time[6] & 0xFF); // Year

    return 0; // :)
}

/**
 * \brief           Checks the RTC for the alarm interrupt flag. If the flag is set, this clears it.
 * \return          `1` if the alarm flag was set, `0` if the flag was not set.
 */
uint8_t RTC_check_alarm_flag(void) {
    uint8_t alarm = (read_from_RTC(0x01));
    if (alarm &  0b01000000) write_to_RTC(0x01, alarm & 0b10111111); // Clear the flag
    return (alarm & 0b01000000);
}

/**
 * \brief           Checks the RTC for the timer interrupt flag. If the flag is set, this clears it. This could mean that either the Minute interrupt, Half Minute interrupt, OR from a normal Timer interrupt.
 * \return          `1` if the timer flag was set, `0` if the flag was not set.
 */
uint8_t RTC_check_timer_flag(void) {
    uint8_t alarm = (read_from_RTC(0x01));
    if (alarm &  0b00001000) write_to_RTC(0x01, alarm & 0b11110111); // Clear the flag
    return (alarm & 0b00001000);
}