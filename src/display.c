
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include "display.h"

uint8_t writeToDisplay(uint8_t reg, uint8_t val) {

	while (!device_is_ready(display_i2c.bus));
	uint8_t config[2] = {reg, val};
	int ret = i2c_write_dt(&display_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	return 0;
}

uint8_t readFromDisplay(uint8_t reg) {

	while (!device_is_ready(display_i2c.bus));
	uint8_t config[1] = {reg};
	int ret = i2c_write_dt(&display_i2c, config, sizeof(config));
	if(ret != 0){
		return 1;
	}

	uint8_t data;
	ret = i2c_read_dt(&display_i2c, &data, sizeof(data));
	if(ret != 0){
		return 1;
	}

	return data;
}

int Display_init() {

	int ret = 0;
  	if (!gpio_is_ready_dt(&S1)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&S1, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}
	
	if (!gpio_is_ready_dt(&S2)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&S2, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	if (!gpio_is_ready_dt(&S3)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&S3, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}
	
	if (!gpio_is_ready_dt(&S4)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&S4, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	
	ret = writeToDisplay(0x03, 0xFF);  // All outputs
	if (ret != 0) {
		return 0;
	}
	ret = writeToDisplay(0x05, 0xFF);  // All outputs in high state
	if (ret != 0) {
		return 0;
	}
	ret = writeToDisplay(0x07, 0x00);  // Turn off all tristates (Enable output control)
	if (ret != 0) {
		return 0;
	}
}

void enableSegment(int segment) {
  if (segment == 1) {
	gpio_pin_set_dt(&S2, 0);
	gpio_pin_set_dt(&S3, 0);
	gpio_pin_set_dt(&S4, 0);

	gpio_pin_set_dt(&S1, 1);
  } else if (segment == 2) {
	gpio_pin_set_dt(&S1, 0);
	gpio_pin_set_dt(&S3, 0);
	gpio_pin_set_dt(&S4, 0);

	gpio_pin_set_dt(&S2, 1);
  } else if (segment == 3) {
	gpio_pin_set_dt(&S1, 0);
	gpio_pin_set_dt(&S2, 0);
	gpio_pin_set_dt(&S4, 0);

	gpio_pin_set_dt(&S3, 1);
  } else if (segment == 4) {
	
	gpio_pin_set_dt(&S1, 0);
	gpio_pin_set_dt(&S2, 0);
	gpio_pin_set_dt(&S3, 0);

	gpio_pin_set_dt(&S4, 1);
  } else {
    disableSegments();
  }
}

void disableSegments() {
	gpio_pin_set_dt(&S1, 0);
	gpio_pin_set_dt(&S2, 0);
	gpio_pin_set_dt(&S3, 0);
	gpio_pin_set_dt(&S4, 0);
}

void displayNone() {
	writeToDisplay(0x05, 0xFF);
}

void displayNumberOnSegment(uint8_t num, int segment) {
  displayNone();
  enableSegment(segment);

  // configureDisplay();
  if (num == 0) writeToDisplay(0x05, 0xff - 0b01111110);
  if (num == 1) writeToDisplay(0x05, 0xff - 0b00001100);
  if (num == 2) writeToDisplay(0x05, 0xff - 0b10110110);
  if (num == 3) writeToDisplay(0x05, 0xff - 0b10011110);
  if (num == 4) writeToDisplay(0x05, 0xff - 0b11001100);
  if (num == 5) writeToDisplay(0x05, 0xff - 0b11011010);
  if (num == 6) writeToDisplay(0x05, 0xff - 0b11111010);
  if (num == 7) writeToDisplay(0x05, 0xff - 0b00001110);
  if (num == 8) writeToDisplay(0x05, 0xff - 0b11111110);
  if (num == 9) writeToDisplay(0x05, 0xff - 0b11011110);
  if (num > 9) writeToDisplay(0x05, 0xff - 0b00000000);
}

void displayColon(){
  writeToDisplay(0x05, 0xFF - 0b00000001);
}

void displayTime(int num1, int num2, bool colon, int num3, int num4) {
	displayNumberOnSegment(num1, 1);
	displayNumberOnSegment(num2, 2);

	if (colon) {
		displayColon();
	}

	displayNumberOnSegment(num3, 3);
	displayNumberOnSegment(num4, 4);
}

void displayOff() {
  displayTime(-1, -1, false, -1, -1);
}
