#include "als.h"

uint8_t genericWriteToDisplayALS(uint8_t reg) {
    // This is going to be used only for the COMMAND "register"
    // It is extremely unclear how to do this.
}

uint8_t commandDisplayALS(uint8_t COMMAND) {
    while (!device_is_ready(display_als_i2c.bus));
	uint8_t config[1] = {COMMAND | 0x80}; // This just ensures that the command register is actually addressed.
	int ret = i2c_write_dt(&display_als_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

void DISPLAY_ALS_init() {
    writeToDisplayALS(DISPLAY_ALS_REG_CONTROL, 1);
    writeToDisplayALS(DISPLAY_ALS_REG_TIMING, 0xFF);
    writeToDisplayALS(DISPLAY_ALS_REG_INTERRUPT, 0x12);
    writeToDisplayALS(DISPLAY_ALS_REG_THH_HIGH, 0x00);
    writeToDisplayALS(DISPLAY_ALS_REG_THH_LOW, 0x00);
    writeToDisplayALS(DISPLAY_ALS_REG_THL_HIGH, 0x00);
    writeToDisplayALS(DISPLAY_ALS_REG_THL_LOW, 0x00);
    // writeToDisplayALS(DISPLAY_ALS_REG_ANALOG, 2);
    DISPLAY_ALS_set_gain(8);
    writeToDisplayALS(DISPLAY_ALS_REG_CONTROL, 3);
    // writeToDisplayALS(DISPLAY_ALS_REG_TIMING, 0x);
    DISPLAY_ALS_set_exposure_ms(10);
    return;

    // writeToDisplayALS(DISPLAY_ALS_REG_TIMING, 0x01);

    // while (1) k_msleep(100);

    // while(1) { k_msleep(500);
    //     readFromDisplayALS(DISPLAY_ALS_REG_CONTROL   );
    //     readFromDisplayALS(DISPLAY_ALS_REG_TIMING    );
    //     readFromDisplayALS(DISPLAY_ALS_REG_INTERRUPT );
    //     readFromDisplayALS(DISPLAY_ALS_REG_THL_LOW   );
    //     readFromDisplayALS(DISPLAY_ALS_REG_THL_HIGH  );
    //     readFromDisplayALS(DISPLAY_ALS_REG_THH_LOW   );
    //     readFromDisplayALS(DISPLAY_ALS_REG_THH_HIGH  );
    //     readFromDisplayALS(DISPLAY_ALS_REG_ANALOG    );
    //     readFromDisplayALS(DISPLAY_ALS_REG_ID        );
    //     readFromDisplayALS(DISPLAY_ALS_REG_DATA0_LOW );
    //     readFromDisplayALS(DISPLAY_ALS_REG_DATA0_HIGH);
    //     readFromDisplayALS(DISPLAY_ALS_REG_DATA1_LOW );
    //     readFromDisplayALS(DISPLAY_ALS_REG_DATA1_HIGH);
    //     readFromDisplayALS(DISPLAY_ALS_REG_TIMER_LOW );
    //     readFromDisplayALS(DISPLAY_ALS_REG_TIMER_HIGH);
    //     readFromDisplayALS(DISPLAY_ALS_REG_ID2       );
    // }
}

uint16_t DISPLAY_ALS_get_brightness(void) {
    // commandDisplayALS(0b11100001);
    uint8_t CONTROL = readFromDisplayALS(0x00);
    if (((CONTROL & 0x20) && (CONTROL & 0x10))  ) {
        uint16_t D0L = readFromDisplayALS(DISPLAY_ALS_REG_DATA0_LOW);
        uint16_t D0H = readFromDisplayALS(DISPLAY_ALS_REG_DATA0_HIGH);
        uint16_t D1L = readFromDisplayALS(DISPLAY_ALS_REG_DATA1_LOW);
        uint16_t D1H = readFromDisplayALS(DISPLAY_ALS_REG_DATA1_HIGH);
    
        
        // genericWriteToDisplayALS(0b11100001); // Should clear interrupts??
        return (D0H << 8) + D0L;
    }

    return 0;
}

uint8_t DISPLAY_ALS_check_ID(void) {
    uint8_t id1 = readFromDisplayALS(DISPLAY_ALS_REG_ID);
    // uint8_t id1 = 0xF0;
    uint8_t id2 = readFromDisplayALS(DISPLAY_ALS_REG_ID2);
    // uint8_t id2 = 0x80;
    return ((id1 & 0xF0) == 0x90) && ((id2 & 0x80));
}

uint8_t writeToDisplayALS(uint8_t reg, uint8_t val) {
	while (!device_is_ready(display_als_i2c.bus));
	uint8_t config[2] = {reg | 0x80, val};
	int ret = i2c_write_dt(&display_als_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}



uint8_t readFromDisplayALS(uint8_t reg) {

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


uint8_t DISPLAY_ALS_set_exposure_ms(float exposure_ms) {
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
    return writeToDisplayALS(DISPLAY_ALS_REG_TIMING, atime );
}

uint8_t DISPLAY_ALS_set_gain(uint8_t gain) {
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
    return writeToDisplayALS(DISPLAY_ALS_REG_ANALOG, data);
}