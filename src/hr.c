#include "hr.h"

void HR_init(void) { 
    LED_driver_init();
    ALS_init();
}

void LED_driver_init(void) {
    write_to_LED_driver(0x5F, 0xC5); // Resets all registers
    write_to_LED_driver(0x01, 0b00100001); // Disables software shut down, disables auto-sleep mode
    write_to_LED_driver(0x02, 0b00011111); // Ensures that all the current outputs CH1-4 are enabled
    write_to_LED_driver(0x04, 0b00101010); // changes the current mode to current level (from PWM & CL) for Ch1-3
    write_to_LED_driver(0x05, 0b00000010); // Changes the current mode to Current level (From PWM & CL) for Ch4
    write_to_LED_driver(0x07, 0b00111111); // Ensures that GCC is set properly
}

void ALS_init(void) {
    write_to_ALS(0x00, 0b00000110); // Enable RGB IR sensing, activating the sensor
    write_to_ALS(0x04, 0b01000000); // 16-bit ADC, 25ms sample rate interval
    write_to_ALS(0x04, 0b00000001); // Set Gain to 3
    write_to_ALS(0x04, 0b01000000); // 16-bit ADC, 25ms sample rate interval
}

void ALS_collect_sample(void) {
    while(read_from_ALS(0x07) & 0b00001000 == 0) k_usleep(10); // Wait for a sample to be collected
    ALS_IR_READING      = read_from_ALS(0x0A) + (read_from_ALS(0x0B) << 8) + (read_from_ALS(0x0C) << 16);
    ALS_GREEN_READING   = read_from_ALS(0x0D) + (read_from_ALS(0x0E) << 8) + (read_from_ALS(0x0F) << 16);
    ALS_BLUE_READING    = read_from_ALS(0x10) + (read_from_ALS(0x11) << 8) + (read_from_ALS(0x12) << 16);
    ALS_RED_READING     = read_from_ALS(0x13) + (read_from_ALS(0x14) << 8) + (read_from_ALS(0x15) << 16);
}

uint32_t ALS_get_recorded_green_value(void) {
    return ALS_GREEN_READING;
}

void LED_driver_enable(void) {
    write_to_LED_driver(0x01, 0b00100001); // Disables software shut down, disables auto-sleep mode
}

void ALS_enable(void) {
    ALS_init();
}

void HR_enable(void) {
    LED_driver_enable();
    ALS_init();
    k_msleep(100);
}

void LED_driver_disable(void) {
    write_to_LED_driver(0x01, 0b00100000); // Enables software shutdown
}

void ALS_disable(void) {
    write_to_ALS(0x00, 0b00000000); // Disable sensor
}

void HR_disable(void) {
    LED_driver_disable();
    ALS_disable();
}

uint8_t write_to_LED_driver(uint8_t reg, uint8_t val) {

	while (!device_is_ready(led_drv_i2c.bus));
	uint8_t config[2] = {reg, val};
	int ret = i2c_write_dt(&led_drv_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

uint8_t read_from_LED_driver(uint8_t reg) {

	while (!device_is_ready(led_drv_i2c.bus));
	uint8_t config[1] = {reg};
	int ret = i2c_write_dt(&led_drv_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	uint8_t data;
	ret = i2c_read_dt(&led_drv_i2c, &data, sizeof(data));
	if(ret != 0){
		return 1;
	}

	return data;
}


uint8_t write_to_ALS(uint8_t reg, uint8_t val) {

	while (!device_is_ready(als_i2c.bus));
	uint8_t config[2] = {reg, val};
	int ret = i2c_write_dt(&als_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

uint8_t read_from_ALS(uint8_t reg) {

	while (!device_is_ready(als_i2c.bus));
	uint8_t config[1] = {reg};
	int ret = i2c_write_dt(&als_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	uint8_t data;
	ret = i2c_read_dt(&als_i2c, &data, sizeof(data));
	if(ret != 0){
		return 1;
	}

	return data;
}


void illuminate_channel(uint8_t channel_number, uint8_t current_mA) {
    uint8_t GCC = 0b00111111; // Default value on power up. Found in register address 0x07
    uint8_t CL  = ((float)current_mA / 30) * (64.0 / GCC) * (256);
    uint8_t addr = 0x10 + channel_number - 1;
    write_to_LED_driver(addr, CL);
    write_to_LED_driver(0x52,0xC5);//Color Update
    // addr = 0x20 + channel_number - 1;
    // write_to_LED_driver(addr, CL);
    // addr = 0x30 + channel_number - 1;
    // write_to_LED_driver(addr, CL);
}

void turn_off_channel(uint8_t channel_number) {
    uint8_t addr = 0x10 + channel_number - 1;
    write_to_LED_driver(addr, 0);
    write_to_LED_driver(0x52,0xC5);//Color Update
    // addr = 0x20 + channel_number - 1;
    // write_to_LED_driver(addr, 0);
    // addr = 0x30 + channel_number - 1;
    // write_to_LED_driver(addr, 0);
}

void illuminate_green(uint8_t current_mA) {
    illuminate_channel(GREEN_CHANNEL, current_mA);
}

void turn_off_green(void) {
    turn_off_channel(GREEN_CHANNEL);
}

void illuminate_red(uint8_t current_mA) {
    illuminate_channel(RED_CHANNEL, current_mA);
}

void turn_off_red(void) {
    turn_off_channel(RED_CHANNEL);
}

//Functions below here may not be used in final setup

void illuminate_blue(uint8_t current_mA) {
    illuminate_channel(BLUE_CHANNEL, current_mA);
}

void turn_off_blue(void) {
    turn_off_channel(BLUE_CHANNEL);
}

void illuminate_IR(uint8_t current_mA){
    illuminate_channel(IR_CHANNEL, current_mA);
}

void turn_off_IR(void){
    turn_off_channel(IR_CHANNEL);
}