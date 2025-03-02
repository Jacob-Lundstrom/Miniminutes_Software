
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

	enableInt1();


	int ret;

	if (!gpio_is_ready_dt(&INT1)) {
		return 0;
	}
	
	ret = gpio_pin_interrupt_configure_dt(&INT1,GPIO_INT_EDGE_TO_ACTIVE);
	if (ret < 0) {
		return 0;
	}
	gpio_init_callback(&pin_cb_data, IMU_wakeup_isr, BIT(INT1.pin));
	gpio_add_callback(INT1.port, &pin_cb_data);
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