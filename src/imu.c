
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#include "imu.h"

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

void enableInt1() {
	// Start registers following AN5038 5.6.5
	writeToIMU(0x20, 0x70); // Modified from 0x74 to 0x70 for low-power operation. Still works with double tap
	writeToIMU(0x25, 0x04);
	writeToIMU(0x30, 0x0C);
	writeToIMU(0x31, 0xEC);
	writeToIMU(0x32, 0xEC);
	writeToIMU(0x33, 0x7F);
	writeToIMU(0x34, 0x80);
	writeToIMU(0x23, 0x08);
	writeToIMU(0x3F, 0x20);
}
