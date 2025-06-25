
#include "imu.h"

static struct gpio_callback pin_cb_data;

/**
 * \brief 			Function to handle writing to the IMU over the I2C bus.
 * \param           reg: Register address to write to
 * \param           val: Value to write to specified register address
 * \return          0 if the I2C write was successful, 1 if there was an error in communication.
 */
uint8_t write_to_IMU(uint8_t reg, uint8_t val) {

	while (!device_is_ready(imu_i2c.bus));
	uint8_t config[2] = {reg, val};
	int ret = i2c_write_dt(&imu_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

/**
 * \brief 			Function to handle reading from the IMU over the I2C bus.
 * \param           reg: Register address in the IMU to read from.
 * \return          Value stored at the requested register address.
 */
uint8_t read_from_IMU(uint8_t reg) {

	while (!device_is_ready(imu_i2c.bus));
	uint8_t config[1] = {reg};
	int ret = i2c_write_dt(&imu_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	uint8_t data;
	ret = i2c_read_dt(&imu_i2c, &data, sizeof(data));
	if(ret != 0){
		return 1;
	}

	return data;
}

/**
 * \brief 			Initializes the IMU to detect taps, and initializes an interrupt for tap detections.
 */
void IMU_init(void) {

	int ret;

	if (!gpio_is_ready_dt(&INT1)) {
		return 0;
	}

	
	ret = gpio_pin_configure_dt(&INT1, GPIO_PULL_UP);
	if (ret < 0) {
		return 0;
	}
	
	ret = gpio_pin_configure_dt(&INT1, GPIO_INPUT);
	if (ret < 0) {
		return 0;
	}
	
	ret = gpio_pin_interrupt_configure_dt(&INT1,GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	gpio_init_callback(&pin_cb_data, IMU_wakeup_isr, BIT(INT1.pin));
	gpio_add_callback(INT1.port, &pin_cb_data);


#ifdef __LIS2DUX12	
	write_to_IMU(0x3E, 1); // Exits deep-power down
	k_msleep(25);

	uint8_t who_am_i = read_from_IMU(0x0f);

	if (who_am_i != 0b01000111) {
		while (1) k_msleep(10);
	}

	// Initiate a software reset
	write_to_IMU(0x10, 0b00100000);
	while (read_from_IMU(0x10) & 0b00100000) k_msleep(10);
	
	write_to_IMU(0x13, 0b00100000); // CTRL4
	write_to_IMU(0x10, 0b00011111); // CTRL1
	
	// write_to_IMU(0x17, 0b00000001); // Enables interrupts, sets interrupt mode to level
	// write_to_IMU(0x10, 0b00010111); // Enables wake-up events on X, Y and Z
	
	write_to_IMU(0x6F, 0b11000101); // axis, inverted_peak_time
	write_to_IMU(0x70, 0b00101000); // Sets pre_still_ths and the post_still_time
	write_to_IMU(0x71, 0b00001000); // shock_wait_time
	write_to_IMU(0x72, 0b10000100); // post_still_ths, latency	
	write_to_IMU(0x73, 0b10001000); // Wait_end_latency, peak_ths
	write_to_IMU(0x74, 0b11100000); // single_tap_on, double_tap_on, triple_tap_on, rebound
	write_to_IMU(0x75, 0b00001010); // pre_still_start, pre_still_n

	write_to_IMU(0x10, read_from_IMU(0x10) | 0b00010000);
	write_to_IMU(0x11, read_from_IMU(0x11));

	write_to_IMU(0x1F, read_from_IMU(0x1F) | 0b00001000); // Enable tap interrupts on INT1

	write_to_IMU(0x17, 0b00000001);

	write_to_IMU(0x0C, 0b00111110); // Configures pulling resistors, sets interrupt level, sets open-drain on INT1
	write_to_IMU(0x14, 0b10100010); // ODR 400 Hz, 8G FS
	write_to_IMU(0x12, 0b00000000); // HP disabled

	read_from_IMU(0x24); // Ensure that we read and clear all interrupt flags
#endif

#ifdef __LIS2DW12
	write_to_IMU(0x20, 0x60); // Enables low power mode 12-bit, 200 Hz ODR
	
	// This is an important one to lower the TDP
	write_to_IMU(0x25, 0x00); // Min digital LP cutoff frequency, Sets full scale value to 4g, enables LP filtering, disables low-noise config
	
	write_to_IMU(0x30, 0x10); // Disables 4D/6D, zets x tap threshold to ~2g
	write_to_IMU(0x31, 0xB0); // Tap priority level Z -> X -> Y, y tap threshold (not used)
	write_to_IMU(0x32, 0xB0); // Enables tap on X, Z; sets z tap threshold to ~2g
	write_to_IMU(0x33, 0x7F); // Sets the maximum time gap between taps for double tap to ~1s
							// maximum quiet time and maximum over-threshold event time
	write_to_IMU(0x34, 0x80); // Enables double tap wakeup
	write_to_IMU(0x23, 0x08); // Sets INT1 to trigger on Double Tap
	write_to_IMU(0x3F, 0x20); // Enables interrupts
#endif
}