#include "pwr.h"


#define I2C0_PWR_NODE DT_NODELABEL(bq25188)
static const struct i2c_dt_spec pwr_i2c = I2C_DT_SPEC_GET(I2C0_PWR_NODE);

static struct gpio_callback chrg_stat_pin_cb_data;

uint8_t write_to_pwr(uint8_t reg, uint8_t val) {

	while (!device_is_ready(pwr_i2c.bus));
	uint8_t config[2] = {reg, val};
	int ret = i2c_write_dt(&pwr_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}


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
}


int PWR_init(void) {
	int ret;

	if (!gpio_is_ready_dt(&CHRG_STAT)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CHRG_STAT, GPIO_PULL_UP);
	ret = gpio_pin_configure_dt(&CHRG_STAT, GPIO_INPUT);
	if (ret < 0) {
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&CHRG_STAT,GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	gpio_init_callback(&chrg_stat_pin_cb_data, PWR_wakeup_isr, BIT(CHRG_STAT.pin));
	ret = gpio_add_callback(CHRG_STAT.port, &chrg_stat_pin_cb_data);

	if (ret != 0) {
		while(1) {
			display_error(99);
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

	return 0;
}

int PWR_disable_charge(){
	// Will return -1 if we cannot safely disable charging
	// This is actually to disable the power path management


	write_to_pwr(0x0A, 0b00000111); // V_SYS = V_BATT
	return 0;
}

int PWR_enable_charge() {

	
	write_to_pwr(0x0A, 0b00000011); // V_SYS = V_INDPM
	return 0;
}


bool PWR_get_is_charging() {
	return (read_from_pwr(0x00) & (0b01100000)); //(00 -> not charging, 01 -> CC, 10 -> CV, 11 -> Charge Done)
}

uint32_t read_battery_voltage(void) {
	uint32_t avg = 0;
	int rdgs = 1;

	int err = 0;
	// err = PWR_disable_charge();
	if (err < 0) {
		return BATTERY_MIN_VOLTAGE_MV;
	}

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
	if (rdgs == 0) return 0;
	return (avg / rdgs) * BATTERY_ADC_SCALE_FACTOR;
}

uint8_t get_battery_percentage(uint32_t battery_mv){
	return 100 * (battery_mv - BATTERY_MIN_VOLTAGE_MV) / 
	(BATTERY_MAX_VOLTAGE_MV - BATTERY_MIN_VOLTAGE_MV);
}