
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#include "main.h"
#include "imu.h"

static struct gpio_callback pin_cb_data;

uint8_t writeToIMU(uint8_t reg, uint8_t val) {

	while (!device_is_ready(imu_i2c.bus));
	uint8_t config[2] = {reg, val};
	int ret = i2c_write_dt(&imu_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

uint8_t readFromIMU(uint8_t reg) {

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

void IMU_init(void) {

	int ret;

	if (!gpio_is_ready_dt(&INT1)) {
		return 0;
	}

	// ret = gpio_pin_configure_dt(&INT1, GPIO_PULL_UP);
	// if (ret < 0) {
	// 	return 0;
	// }
	
	// ret = gpio_pin_configure_dt(&INT1, GPIO_INPUT);
	// if (ret < 0) {
	// 	return 0;
	// }
	
	ret = gpio_pin_interrupt_configure_dt(&INT1,GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	gpio_init_callback(&pin_cb_data, IMU_wakeup_isr, BIT(INT1.pin));
	gpio_add_callback(INT1.port, &pin_cb_data);

	
	enableInt1();
}

#ifdef __LIS2DW12
void enableInt1() {
	writeToIMU(0x20, 0x60); // Enables low power mode 12-bit, 200 Hz ODR
	
	// This is an important one to lower the TDP
	writeToIMU(0x25, 0x00); // Min digital LP cutoff frequency, Sets full scale value to 4g, enables LP filtering, disables low-noise config
	
	writeToIMU(0x30, 0x10); // Disables 4D/6D, zets x tap threshold to ~2g
	writeToIMU(0x31, 0xB0); // Tap priority level Z -> X -> Y, y tap threshold (not used)
	writeToIMU(0x32, 0xB0); // Enables tap on X, Z; sets z tap threshold to ~2g
	writeToIMU(0x33, 0x7F); // Sets the maximum time gap between taps for double tap to ~1s
							// maximum quiet time and maximum over-threshold event time
	writeToIMU(0x34, 0x80); // Enables double tap wakeup
	writeToIMU(0x23, 0x08); // Sets INT1 to trigger on Double Tap
	writeToIMU(0x3F, 0x20); // Enables interrupts
}
#endif

#ifdef __LIS2DUX12
void enableInt1() {
#if 0
	writeToIMU(0x3E, 1); // Enables soft-power down
	k_msleep(30);

	writeToIMU(0x14, 0b00000000);

	writeToIMU(0x6F, 0b11000101); // Enables tap detection on Z, and sets inverted peak time to 5 samples

	writeToIMU(0x0C, 0b00111110); // Configures pulling resistors (Improves performance)
	writeToIMU(0x10, 0b10010111); // Enables smart power, enables wake-up events on X, Y and Z
	writeToIMU(0x12, 0b00000000); // Ensures that IMU is configured for Low-power mode

	writeToIMU(0x17, 0b00000001); // Enables interrupts

	writeToIMU(0x1C, 0b00000100); // Enables sleep and sets wakeup threshold

	writeToIMU(0x1F, 0b00001000); // Enables routing all tap interrupts to INT1

	writeToIMU(0x70, 0b01111111); // [7:4] Sets threshold for stationary condition [3:0] sets number of samples in stationary condition
	writeToIMU(0x71, 0b11111111); // [7:6] Sets high count for number of samples in stationary condition, [5:0] sets number of samples to wait for shock to stop
	writeToIMU(0x72, 0b01110010); // [7:4] Sets threshold for stationary condition after shock [3:0] sets max number of samples between taps to 2*32 = 64 samples
	writeToIMU(0x73, 0b00000111); // [4:0] Sets peak threshold to ~2g
	writeToIMU(0x74, 0b11100000); // Enable double tap detection
	writeToIMU(0x75, 0b00001111); // Configures number of still samples before a shock

	// This might have to be done last?
	writeToIMU(0x14, 0b00110101); // Sets ODR to 25 Hz ULP, BW to 12.5 Hz, and FS = +- 4G
	
	// writeToIMU(0x14, 0b00000000); // Power down
#endif

#if 1

	writeToIMU(0x3E, 1); // Exits deep-power down
	k_msleep(25);

	uint8_t who_am_i = readFromIMU(0x0f);

	if (who_am_i != 0b01000111) {
		while (1) k_msleep(10);
	}

	// Initiate a software reset
	writeToIMU(0x10, 0b00100000);
	while (readFromIMU(0x10) & 0b00100000) k_msleep(10);
	
	writeToIMU(0x13, 0b00100000); // CTRL4
	writeToIMU(0x10, 0b00011111); // CTRL1
	
	// writeToIMU(0x17, 0b00000001); // Enables interrupts, sets interrupt mode to level
	// writeToIMU(0x10, 0b00010111); // Enables wake-up events on X, Y and Z
	
	writeToIMU(0x6F, 0b11000101); // axis, inverted_peak_time
	writeToIMU(0x70, 0b00101000); // Sets pre_still_ths and the post_still_time
	writeToIMU(0x71, 0b00001000); // shock_wait_time
	writeToIMU(0x72, 0b10000100); // post_still_ths, latency	
	writeToIMU(0x73, 0b10001000); // Wait_end_latency, peak_ths
	writeToIMU(0x74, 0b11100000); // single_tap_on, double_tap_on, triple_tap_on, rebound
	writeToIMU(0x75, 0b00001010); // pre_still_start, pre_still_n

	writeToIMU(0x10, readFromIMU(0x10) | 0b00010000);
	writeToIMU(0x11, readFromIMU(0x11));

	writeToIMU(0x1F, readFromIMU(0x1F) | 0b00001000); // Enable tap interrupts on INT1

	writeToIMU(0x17, 0b00000001);

	// writeToIMU(0x0C, 0b00111110); // Configures pulling resistors, sets interrupt level, sets open-drain on INT1
	writeToIMU(0x14, 0b10100010); // ODR 400 Hz, 8G FS
	writeToIMU(0x12, 0b00000000); // HP disabled

	readFromIMU(0x24); // Ensure that we read and clear all interrupt flags
#endif
}
#endif