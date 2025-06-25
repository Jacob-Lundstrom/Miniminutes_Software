#include "display_als.h"

/**
 * \brief 			Sends a byte of information to the COMMAND register in the ALS.
 * \param 			COMMAND: Byte to be written to COMMAND register.
 */
uint8_t command_Display_ALS(uint8_t COMMAND) {
    while (!device_is_ready(display_als_i2c.bus));
	uint8_t config[1] = {COMMAND | 0x80}; // This just ensures that the command register is actually addressed.
	int ret = i2c_write_dt(&display_als_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

/**
 * \brief 			Initializes the ALS with some acceptable settings to get the current brightness.
 */
void Display_ALS_init() {
    write_to_Display_ALS(DISPLAY_ALS_REG_CONTROL, 1);
    write_to_Display_ALS(DISPLAY_ALS_REG_TIMING, 0xFF);
    write_to_Display_ALS(DISPLAY_ALS_REG_INTERRUPT, 0x12);
    write_to_Display_ALS(DISPLAY_ALS_REG_THH_HIGH, 0x00);
    write_to_Display_ALS(DISPLAY_ALS_REG_THH_LOW, 0x00);
    write_to_Display_ALS(DISPLAY_ALS_REG_THL_HIGH, 0x00);
    write_to_Display_ALS(DISPLAY_ALS_REG_THL_LOW, 0x00);
    Display_ALS_set_gain(8);
    write_to_Display_ALS(DISPLAY_ALS_REG_CONTROL, 3);
    Display_ALS_set_exposure_ms(10);
    write_to_Display_ALS(DISPLAY_ALS_REG_INTERRUPT, 0x00); // Disables interrupts
    return;

    // For debugging:
    // while(1) { k_msleep(500);
    //     read_from_Display_ALS(DISPLAY_ALS_REG_CONTROL   );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_TIMING    );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_INTERRUPT );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_THL_LOW   );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_THL_HIGH  );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_THH_LOW   );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_THH_HIGH  );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_ANALOG    );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_ID        );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_DATA0_LOW );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_DATA0_HIGH);
    //     read_from_Display_ALS(DISPLAY_ALS_REG_DATA1_LOW );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_DATA1_HIGH);
    //     read_from_Display_ALS(DISPLAY_ALS_REG_TIMER_LOW );
    //     read_from_Display_ALS(DISPLAY_ALS_REG_TIMER_HIGH);
    //     read_from_Display_ALS(DISPLAY_ALS_REG_ID2       );
    // }
}

/**
 * \brief 			Gets the intensity of illumination onto the sensor.
 * \return          Illumination of the photodiode array in lux. -1 corresponds to an invalid reading.
 */
int32_t Display_ALS_get_brightness(void) {
    // command_Display_ALS(0b11100001);
    uint8_t CONTROL = read_from_Display_ALS(0x00);
    if (((CONTROL & 0x20) && (CONTROL & 0x10))  ) {
        uint16_t D0L = read_from_Display_ALS(DISPLAY_ALS_REG_DATA0_LOW);
        uint16_t D0H = read_from_Display_ALS(DISPLAY_ALS_REG_DATA0_HIGH);
        uint16_t D1L = read_from_Display_ALS(DISPLAY_ALS_REG_DATA1_LOW);
        uint16_t D1H = read_from_Display_ALS(DISPLAY_ALS_REG_DATA1_HIGH);
    
        
        // genericWriteToDisplayALS(0b11100001); // Should clear interrupts??
        return (D0H << 8) + D0L;
    }

    return -1;
}

/**
 * \brief 			Checks the identity of the ALS sensor.
 * \return          True if the sensor's ID is correct, False otherwise.
 */
uint8_t Display_ALS_check_ID(void) {
    uint8_t id1 = read_from_Display_ALS(DISPLAY_ALS_REG_ID);
    uint8_t id2 = read_from_Display_ALS(DISPLAY_ALS_REG_ID2);
    return ((id1 & 0xF0) == 0x90) && ((id2 & 0x80));
}

/**
 * \brief 			Function to handle writing to the ALS over the I2C bus.
 * \param           reg: Register address to write to
 * \param           val: Value to write to specified register address
 * \return          0 if the I2C write was successful, 1 if there was an error in communication.
 */
uint8_t write_to_Display_ALS(uint8_t reg, uint8_t val) {
	while (!device_is_ready(display_als_i2c.bus));
	uint8_t config[2] = {reg | 0x80, val};
	int ret = i2c_write_dt(&display_als_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}


/**
 * \brief 			Function to handle reading from the Display ALS over the I2C bus.
 * \param           reg: Register address in the Display ALS to read from.
 * \return          Value stored at the requested register address.
 */
uint8_t read_from_Display_ALS(uint8_t reg) {

	while (!device_is_ready(display_als_i2c.bus));
	uint8_t config[1] = {reg | 0x80};
	int ret = i2c_write_dt(&display_als_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	uint8_t data;
	ret = i2c_read_dt(&display_als_i2c, &data, sizeof(data));
	if(ret != 0){
		return 1;
	}

	return data;
}

/**
 * \brief 			Sets the exposure time of the display in ms to as close as possible to the requested value.
 * \param           exposure_ms: The desired exposure time, in milliseconds. Values range from 2.7ms to 688.5 ms.
 * \return          0 if the I2C write was successful, 1 if there was an error in communication.
 */
uint8_t Display_ALS_set_exposure_ms(float exposure_ms) {
    uint8_t atime = 0;
    if ( exposure_ms >= 688.5 ) 
    {
        atime = 1;
    }
    else if ( exposure_ms <= 2.7 )
    {
        atime = 0xFF;
    }
    else
    {
        atime = 0xFF - ( uint8_t ) ( exposure_ms / 2.7 );
    }
    return write_to_Display_ALS(DISPLAY_ALS_REG_TIMING, atime );
}

/**
 * \brief 			Sets the gain of the Display ALS.
 * \param           gain: The desired sensor gaine. Valid settings are 1, 8, 16, or 111 (high-gain mode).
 * \return          0 if the I2C write was successful, 1 if there was an error in communication or an invalid gain was requested.
 */
uint8_t Display_ALS_set_gain(uint8_t gain) {
    uint8_t data = 0;
    switch ( gain )
    {
        case 1:
        {
            data = 0;
            break;
        }
        case 8:
        {
            data = 0b1;
            break;
        }
        case 16:
        {
            data = 0b10;
            break;
        }
        case 111:
        {
            data = 0b11;
            break;
        }
        default:
        {
            return 1;
        }
    }
    return write_to_Display_ALS(DISPLAY_ALS_REG_ANALOG, data);
}


uint8_t Display_ALS_enable(void) {
    // write_to_Display_ALS(DISPLAY_ALS_REG_CONTROL, read_from_Display_ALS(DISPLAY_ALS_REG_CONTROL) | (0b00000010));
    Display_ALS_init();
}

uint8_t Display_ALS_disable(void) {
    // write_to_Display_ALS(DISPLAY_ALS_REG_TIMING    , 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_INTERRUPT , 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_THL_LOW   , 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_THL_HIGH  , 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_THH_LOW   , 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_THH_HIGH  , 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_ANALOG    , 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_ID        , 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_DATA0_LOW , 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_DATA0_HIGH, 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_DATA1_LOW , 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_DATA1_HIGH, 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_TIMER_LOW , 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_TIMER_HIGH, 0x00);
    // write_to_Display_ALS(DISPLAY_ALS_REG_ID2       , 0x00);

    // write_to_Display_ALS(DISPLAY_ALS_REG_CONTROL, 0x00); // Power down
    // Now there should be no difference between a power cycle and a "disable"
}