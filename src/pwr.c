#include "pwr.h"

static struct gpio_callback chrg_stat_pin_cb_data;

/**
 * \brief 			Function to handle writing to the BMS over the I2C bus.
 * \param           reg: Register address to write to
 * \param           val: Value to write to specified register address
 * \return          0 if the I2C write was successful, 1 if there was an error in communication.
 */
uint8_t write_to_pwr(uint8_t reg, uint8_t val) {

	while (!device_is_ready(pwr_i2c.bus));
	uint8_t config[2] = {reg, val};
	int ret = i2c_write_dt(&pwr_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

/**
 * \brief 			Function to handle reading from the BMS over the I2C bus.
 * \param           reg: Register address in the BMS to read from.
 * \return          Value stored at the requested register address.
 */
uint8_t read_from_pwr(uint8_t reg) {

	while (!device_is_ready(pwr_i2c.bus));
	uint8_t config[1] = {reg};
	int ret = i2c_write_dt(&pwr_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	uint8_t data;
	ret = i2c_read_dt(&pwr_i2c, &data, sizeof(data));
	if(ret != 0){
		return 1;
	}

	return data;
}


int err;
uint32_t count = 0;
uint16_t buf;
struct adc_sequence sequence = {
	.buffer = &buf,
	/* buffer size in bytes, not number of samples */
	.buffer_size = sizeof(buf),
};

/**
 * \brief		Initializes the ADC channel used for measuring the battery voltage.
 * \return		0 on success, error number on failure.
 */
int ADC_init(void) {
	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!adc_is_ready_dt(&adc_channels[i])) {
			// printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
			return 0;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			// printk("Could not setup channel #%d (%d)\n", i, err);
			return 0;
		}
	}
	return 1;
}

/**
 * \brief		Initializes the BMS.
 * \return		1 on success, 0 on failure.
 */
int PWR_init(void) {

	int ret;

	if (!gpio_is_ready_dt(&CHRG_RTC_INT)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CHRG_RTC_INT, GPIO_PULL_UP);
	if (ret < 0) {
		return 0;
	}
	
	ret = gpio_pin_configure_dt(&CHRG_RTC_INT, GPIO_INPUT);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&CHRG_RTC_INT,GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	gpio_init_callback(&chrg_stat_pin_cb_data, common_interrupt_isr, BIT(CHRG_RTC_INT.pin));
	ret = gpio_add_callback(CHRG_RTC_INT.port, &chrg_stat_pin_cb_data);

	if (ret != 0) {
		while(1) {
			Display_display_error(01);
		}
	}


	// Communicate with the BQ25180/BQ25188
	write_to_pwr(0x03, 0b01000110); // Sets the battery max charge voltage to 4.2V
	write_to_pwr(0x04, 0b00001111); // Sets the battery fast charge current to 20 mA, enables charging
	write_to_pwr(0x05, 0b00010000); // Sets the precharge current to 2x Termination current, Termination current to 5% fast charge current, VINDPM = 4.2V, Thermal limit = 100C
	write_to_pwr(0x06, 0b00010011); // Sets battery discharge limit to 500mA, UVLO to 3.0V, enables Charging status interupts
	write_to_pwr(0x07, 0b10000011); // Disables watchdog

	// DO NOT EDIT THIS LINE
	write_to_pwr(0x0A, 0b00000011); // Changes SYS voltage regulation to battery tracking mode. Disables VDPPM. Enables a watchdog for I2C transactions after VIN applied.
	// This is an absolute requirement to ensure that the system does not become permanently bricked.
	// In the case where the SYS pin is changed to floating or pulled down, the system will be unpowered.
	// If the I2C Watchdog for after VIN enabled transactions is diabled, the system cannot recover.
	// Therefore, the 

	write_to_pwr(0x0C, 0b11100000); // Enables only necessary PG / VINOVP interrupt

	return 1;
}

/**
 * \brief		Causes the BMS to disconnect the input voltage from the rest of the system. System voltage is supplied from battery only.
 * \return		0 on success, 1 on failure
 */
int PWR_disconnect_from_charger(void){
	return write_to_pwr(0x0A, (read_from_pwr(0x0A) & !(0b11 << 2)) | (0b1 << 2)); // V_SYS = V_BATT
}

/**
 * \brief		Causes the BMS to connect the input voltage, if present, to the rest of the system. System voltage is supplied from input if present, and from battery if not.
 * \return		0 on success, 1 on failure
 */
int PWR_reconnect_to_charger(void) {
	return write_to_pwr(0x0A, (read_from_pwr(0x0A) & !(0b11 << 2))); // V_SYS = V_INDPM
}

/**
 * \brief		Determines if there is a supplied voltage on VIN
 * \return		1 if VIN present, 0 otherwise.
 */
bool PWR_get_is_on_charger(void) {
	return (read_from_pwr(0x00 * 0b1)); // VIN Power good
}

/**
 * \brief		Determines the charge progress of the battery.
 * \return		`0b00` for Not Charging, `0b01` for Constant Current charging, `0b10` for Constant Voltage Charging, or `0b11` for Charge Done.
 */
uint8_t PWR_get_charge_status(void) {
	return ((read_from_pwr(0x00) & (0b01100000)) >> 5); //(00 -> not charging, 01 -> CC, 10 -> CV, 11 -> Charge Done)
}

/**
 * \brief		Samples the battery voltage 100 times, then returns the average.
 * \return		Average of 100 samples of ADC measurements, in mV
 */
uint32_t read_battery_voltage(void) {
	uint32_t avg = 0;
	int rdgs = 100;

	int err = 0;
	// err = PWR_disconnect_from_charger();
	if (err < 0) {
		return BATTERY_MIN_VOLTAGE_MV;
	}

	// BEGIN MODIFICATION FOR LOAD SWITCH
	int ret;
	
	// I have to do this for some reason? I think its because setting pull-up to this pin changes it to open drain config for output.
	ret = gpio_pin_configure_dt(&CHRG_RTC_INT, GPIO_PIN_CNF_PULL_Disabled); 

	ret += gpio_pin_configure_dt(&CHRG_RTC_INT, GPIO_OPEN_DRAIN);
	ret += gpio_pin_set_dt(&CHRG_RTC_INT, 1); 
	ret += gpio_pin_configure_dt(&CHRG_RTC_INT, GPIO_OUTPUT);

	k_msleep(30);

	if (ret < 0) {
		Display_display_error(33);
	}
	// END MODIFICATION FOR LOAD SWITCH

	// printk("ADC reading[%u]:\n", count++);
	for(int k = 0; k < rdgs; k++) {
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		int32_t val_mv;

		printk("- %s, channel %d: ",
				adc_channels[i].dev->name,
				adc_channels[i].channel_id);

		(void)adc_sequence_init_dt(&adc_channels[i], &sequence);

		err = adc_read_dt(&adc_channels[i], &sequence);
		if (err < 0) {
			// printk("Could not read (%d)\n", err);
			rdgs--;
			continue;
		}

		/*
			* If using differential mode, the 16 bit value
			* in the ADC sample buffer should be a signed 2's
			* complement value.
			*/
		if (adc_channels[i].channel_cfg.differential) {
			val_mv = (int32_t)((int16_t)buf);
		} else {
			val_mv = (int32_t)buf;
		}
		// printk("%"PRId32, val_mv);
		err = adc_raw_to_millivolts_dt(&adc_channels[i],
							&val_mv);
		/* conversion to mV may not be supported, skip if not */
		if (err < 0) {
			// printk(" (value in mV not available)\n");
		} else {
			avg += val_mv;
		}
	}
	}
	
	// BEGIN MODIFICATION FOR LOAD SWITCH
	PWR_init();
	// At this point, we also need to check for missed interrupts.
	// Check the BMS and the RTC.
	if (RTC_check_alarm_flag()) continue_showing_time();
	if (RTC_check_timer_flag()) continue_showing_battery_voltage();
	is_charging = PWR_get_is_on_charger();

	// END MODIFICATION FOR LOAD SWITCH
	
	if (rdgs == 0) return 0;
	return (avg / rdgs) * BATTERY_ADC_SCALE_FACTOR;
}

/**
 * \brief		Converts a given battery voltage in mV to a battery charge percentage.
 * \return		Nearest integer percent of battery charge remaining.
 */
uint8_t get_battery_percentage(uint32_t battery_mv){
	return 100 * (battery_mv - BATTERY_MIN_VOLTAGE_MV) / 
	(BATTERY_MAX_VOLTAGE_MV - BATTERY_MIN_VOLTAGE_MV);
}