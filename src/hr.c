#include "hr.h"
/**
 * \brief       Initializes all of the IC's associated with HR measurement (LED Driver, HR ALS)
 */
void HR_init(void) { 
    HR_LED_driver_init();
    HR_ALS_init();
}

/**
 * \brief       Initializes the LED driver used to illuminate LEDs for HR measurement.
 */
void HR_LED_driver_init(void) {
    write_to_HR_LED_driver(0x5F, 0xC5); // Resets all registers
    write_to_HR_LED_driver(0x01, 0b00100001); // Disables software shut down, disables auto-sleep mode
    write_to_HR_LED_driver(0x02, 0b00011111); // Ensures that all the current outputs CH1-4 are enabled
    write_to_HR_LED_driver(0x04, 0b00101010); // changes the current mode to current level (from PWM & CL) for Ch1-3
    write_to_HR_LED_driver(0x05, 0b00000010); // Changes the current mode to Current level (From PWM & CL) for Ch4
    write_to_HR_LED_driver(0x07, 0b00111111); // Ensures that GCC is set properly
}

/**
 * \brief       Initializes the HR ALS used to measure changes in reflected light.
 */
void HR_ALS_init(void) {
    write_to_HR_ALS(0x00, 0b00000110); // Enable RGB IR sensing, activating the sensor


    write_to_HR_ALS(0x04, 0b01010000); // 13-bit ADC, 25ms sample rate interval
    // write_to_HR_ALS(0x04, 0b01000000); // 16-bit ADC, 25ms sample rate interval
    // write_to_HR_ALS(0x04, 0b00000111); // 20-bit resolution, 2000 ms sample rate interval

    // write_to_HR_ALS(0x05, 0b00000000); // Set Gain to 1
    // write_to_HR_ALS(0x05, 0b00000001); // Set Gain to 3
    // write_to_HR_ALS(0x05, 0b00000010); // Set Gain to 6
    // write_to_HR_ALS(0x05, 0b00000011); // Set Gain to 9
    write_to_HR_ALS(0x05, 0b00000100); // Set Gain to 18
}

/**
 * \brief       Waits until a new sample is collected from the HR ALS, then stores the reading from each channel.
 */
void HR_ALS_collect_sample(void) {
    while((read_from_HR_ALS(0x07) & 0b00001000) == 0) k_usleep(100); // Wait for a sample to be collected
    ALS_IR_READING      = read_from_HR_ALS(0x0A) + (read_from_HR_ALS(0x0B) << 8) + (read_from_HR_ALS(0x0C) << 16);
    ALS_GREEN_READING   = read_from_HR_ALS(0x0D) + (read_from_HR_ALS(0x0E) << 8) + (read_from_HR_ALS(0x0F) << 16);
    ALS_BLUE_READING    = read_from_HR_ALS(0x10) + (read_from_HR_ALS(0x11) << 8) + (read_from_HR_ALS(0x12) << 16);
    ALS_RED_READING     = read_from_HR_ALS(0x13) + (read_from_HR_ALS(0x14) << 8) + (read_from_HR_ALS(0x15) << 16);
}

/**
 * \brief       Gets the last recorded ADC counts in the HR ALS's red channel.
 * \return      Red Channel ADC count
 */
uint32_t HR_ALS_get_recorded_red_value(void) {
    return ALS_RED_READING;
}

/**
 * \brief       Gets the last recorded ADC counts in the HR ALS's green channel.
 * \return      Green Channel ADC count
 */
uint32_t HR_ALS_get_recorded_green_value(void) {
    return ALS_GREEN_READING;
}

/**
 * \brief       Gets the last recorded ADC counts in the HR ALS's blue channel.
 * \return      Blue Channel ADC count
 */
uint32_t HR_ALS_get_recorded_blue_value(void) {
    return ALS_BLUE_READING;
}


/**
 * \brief       Gets the last recorded ADC counts in the HR ALS's IR channel.
 * \return      IR Channel ADC count
 */
uint32_t HR_ALS_get_recorded_IR_value(void) {
    return ALS_IR_READING;
}

/**
 * \brief       Causes the LED driver to exit software shut down, and disables auto-sleep.
 */
void HR_LED_driver_enable(void) {
    write_to_HR_LED_driver(0x01, 0b00100001); // Disables software shut down, disables auto-sleep mode
}

/**
 * \brief       Re-initializes the HR ALS to exit a shut-down state.
 */
void HR_ALS_enable(void) {
    HR_ALS_init();
}

/**
 * \brief       Re-initializes all the ICs associated with HR measuremetn to exit a shut-down state.
 */
void HR_enable(void) {
    HR_LED_driver_enable();
    HR_ALS_init();
    k_msleep(100);
}

/**
 * \brief       Sets the LED driver to a software shutdown state to lower current consumption.
 */
void LED_driver_disable(void) {
    write_to_HR_LED_driver(0x01, 0b00100000); // Enables software shutdown
}

/**
 * \brief       Disables the HR ALS to lower current consumptoin
 */
void ALS_disable(void) {
    write_to_HR_ALS(0x00, 0b00000000); // Disable sensor
}

/**
 * \brief       Disables all ICs associated with HR measurement to lower current draw.
 */
void HR_disable(void) {
    LED_driver_disable();
    ALS_disable();
}

/**
 * \brief 			Function to handle writing to the HR LED driver over the I2C bus.
 * \return          0 if the I2C write was successful, 1 if there was an error in communication.
 */
uint8_t write_to_HR_LED_driver(uint8_t reg, uint8_t val) {

	while (!device_is_ready(led_drv_i2c.bus));
	uint8_t config[2] = {reg, val};
	int ret = i2c_write_dt(&led_drv_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

/**
 * \brief 			Function to handle reading from the HR LED driver over the I2C bus.
 * \param           reg: Register address in the HR LED driver to read from.
 * \return          Value stored at the requested register address.
 */
uint8_t read_from_HR_LED_driver(uint8_t reg) {

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

/**
 * \brief 			Function to handle writing to the HR ALS over the I2C bus.
 * \return          0 if the I2C write was successful, 1 if there was an error in communication.
 */
uint8_t write_to_HR_ALS(uint8_t reg, uint8_t val) {

	while (!device_is_ready(als_i2c.bus));
	uint8_t config[2] = {reg, val};
	int ret = i2c_write_dt(&als_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

/**
 * \brief 			Function to handle reading from the HR ALS over the I2C bus.
 * \param           reg: Register address in the HR ALS to read from.
 * \return          Value stored at the requested register address.
 */
uint8_t read_from_HR_ALS(uint8_t reg) {

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


/**
 * \brief       Collects a single data point of reflected red illumination, with a background subtraction.
 * \return      HR ALS's red channel ADC reading after subtraction of background.
 */
uint32_t HR_collect_red_sample(void) {
    uint32_t rdg = 0;

    // Illuminated
    HR_LED_illuminate_red(RED_CURRENT_mA);
    HR_ALS_collect_sample(); 
    rdg += HR_ALS_get_recorded_red_value();

    // Baseline
    HR_LED_turn_off_red();
    HR_ALS_collect_sample(); 
    rdg -= HR_ALS_get_recorded_red_value();

    return rdg;
}


/**
 * \brief       Collects a single data point of reflected green illumination, with a background subtraction.
 * \return      HR ALS's green channel ADC reading after subtraction of background.
 */
uint32_t HR_collect_green_sample(void) {

    uint32_t rdg = 0;

    // Illuminated
    HR_LED_illuminate_green(GREEN_CURRENT_mA);
    HR_ALS_collect_sample(); 
    rdg += HR_ALS_get_recorded_green_value();

    // Baseline
    HR_LED_turn_off_green();
    HR_ALS_collect_sample(); 
    rdg -= HR_ALS_get_recorded_green_value();

    return rdg;
}


/**
 * \brief       Collects a single data point of reflected blue illumination, with a background subtraction.
 * \return      HR ALS's blue channel ADC reading after subtraction of background.
 */
uint32_t HR_collect_blue_sample(void) {

    uint32_t rdg = 0;

    // Illuminated
    HR_LED_illuminate_blue(BLUE_CURRENT_mA);
    HR_ALS_collect_sample(); 
    rdg += HR_ALS_get_recorded_blue_value();

    // Baseline
    HR_LED_turn_off_blue();
    HR_ALS_collect_sample(); 
    rdg -= HR_ALS_get_recorded_blue_value();

    return rdg;
}


/**
 * \brief       Collects a single data point of reflected IR illumination, with a background subtraction.
 * \return      HR ALS's IR channel ADC reading after subtraction of background.
 */
uint32_t HR_collect_IR_sample(void) {
    uint32_t rdg = 0;

    // Illuminated
    HR_LED_illuminate_IR(IR_CURRENT_mA);
    HR_ALS_collect_sample(); 
    rdg += HR_ALS_get_recorded_IR_value();

    // Baseline
    HR_LED_turn_off_IR();
    HR_ALS_collect_sample(); 
    rdg -= HR_ALS_get_recorded_IR_value();

    return rdg;
}


/**
 * \brief       Helper method that turns on a selected channel number, driving that channel's led with a requested constant current.
 * \param       channel_number: LED driver channel number to be illuminated
 * \param       current_mA: Targer constant current through the LED, in milliamps (mA)
 */
void HR_LED_illuminate_channel(uint8_t channel_number, uint8_t current_mA) {
    uint8_t GCC = 0b00111111; // Default value on power up. Found in register address 0x07
    uint8_t CL  = ((float)current_mA / 30) * (64.0 / GCC) * (256);
    uint8_t addr = 0x10 + channel_number - 1;
    write_to_HR_LED_driver(addr, CL);
    write_to_HR_LED_driver(0x52,0xC5);//Color Update
}

/**
 * \brief       Helper method to disable a selected channel number.
 * \param       channel_number: LED driver channel number to be disabled.
 */
void HR_LED_turn_off_channel(uint8_t channel_number) {
    uint8_t addr = 0x10 + channel_number - 1;
    write_to_HR_LED_driver(addr, 0);
    write_to_HR_LED_driver(0x52,0xC5);//Color Update
}

/**
 * \brief       Helper method to enable the red LED with a desired constant current
 * \param       current_mA: constant current target for the red LED.
 */
void HR_LED_illuminate_red(uint8_t current_mA) {
    HR_LED_illuminate_channel(RED_CHANNEL, current_mA);
}

/**
 * \brief       Disables the red HR LED.
 */
void HR_LED_turn_off_red(void) {
    HR_LED_turn_off_channel(RED_CHANNEL);
}


/**
 * \brief       Helper method to enable the green LED with a desired constant current
 * \param       current_mA: constant current target for the green LED.
 */
void HR_LED_illuminate_green(uint8_t current_mA) {
    HR_LED_illuminate_channel(GREEN_CHANNEL, current_mA);
}

/**
 * \brief       Disables the green HR LED.
 */
void HR_LED_turn_off_green(void) {
    HR_LED_turn_off_channel(GREEN_CHANNEL);
}


/**
 * \brief       Helper method to enable the blue LED with a desired constant current
 * \param       current_mA: constant current target for the blue LED.
 */
void HR_LED_illuminate_blue(uint8_t current_mA) {
    HR_LED_illuminate_channel(BLUE_CHANNEL, current_mA);
}

/**
 * \brief       Disables the blue HR LED.
 */
void HR_LED_turn_off_blue(void) {
    HR_LED_turn_off_channel(BLUE_CHANNEL);
}


/**
 * \brief       Helper method to enable the IR LED with a desired constant current
 * \param       current_mA: constant current target for the red LED.
 */
void HR_LED_illuminate_IR(uint8_t current_mA){
    HR_LED_illuminate_channel(IR_CHANNEL, current_mA);
}

/**
 * \brief       Disables the IR HR LED.
 */
void HR_LED_turn_off_IR(void){
    HR_LED_turn_off_channel(IR_CHANNEL);
}