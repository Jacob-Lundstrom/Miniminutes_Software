
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

void display_time_seconds(int time_seconds) {
	
	int hr = time_seconds / (60*60) ;
	int min = time_seconds / (60) - 60 * hr;
	
	int dig1 = hr/10;
	int dig2 = hr - dig1 * 10;
	int dig3 = min/10;
	int dig4 = min - dig3 * 10;
	displayTime(dig1, dig2, true, dig3, dig4);
}

void display_time_seconds_mil(int time_seconds, bool military_time) {
	
	int hr = time_seconds / (60*60) ;
	int min = time_seconds / (60) - 60 * hr;
	
	if (!military_time) {
		if (hr == 0){
			hr = 12;
		}
		if (hr > 12) {
			hr -= 12;
		}
	}

	int dig1 = hr/10;
	int dig2 = hr - dig1 * 10;
	int dig3 = min/10;
	int dig4 = min - dig3 * 10;

	if (!military_time) {
		if (dig1 == 0) {
			dig1 = -1; // Don't show the highest hour time if using non-24 hr time.
		}
	}
	displayTime(dig1, dig2, true, dig3, dig4);
}

void display_letter_on_segment(char letter, int segment) {
	displayNone();
	enableSegment(segment);

	// I haven't double checked most of these.
	// Follows the sample given on https://en.wikipedia.org/wiki/Seven-segment_display_character_representations
	if (letter == 'A') writeToDisplay(0x05, 0xff - 0b11101110);
	if (letter == 'B') writeToDisplay(0x05, 0xff - 0b11111110);
	if (letter == 'C') writeToDisplay(0x05, 0xff - 0b01110010);
	if (letter == 'D') writeToDisplay(0x05, 0xff - 0b00111110);
	if (letter == 'E') writeToDisplay(0x05, 0xff - 0b11110010);
	if (letter == 'F') writeToDisplay(0x05, 0xff - 0b11100010);
	if (letter == 'G') writeToDisplay(0x05, 0xff - 0b01111010);
	if (letter == 'H') writeToDisplay(0x05, 0xff - 0b11101100);
	if (letter == 'I') writeToDisplay(0x05, 0xff - 0b01100000);
	if (letter == 'J') writeToDisplay(0x05, 0xff - 0b00111100);
	if (letter == 'K') writeToDisplay(0x05, 0xff - 0b11101010);
	if (letter == 'L') writeToDisplay(0x05, 0xff - 0b01110000);
	if (letter == 'M') writeToDisplay(0x05, 0xff - 0b01010110);
	if (letter == 'N') writeToDisplay(0x05, 0xff - 0b01101110);
	if (letter == 'O') writeToDisplay(0x05, 0xff - 0b01111110);
	if (letter == 'P') writeToDisplay(0x05, 0xff - 0b11100110);
	if (letter == 'Q') writeToDisplay(0x05, 0xff - 0b11010110);
	if (letter == 'R') writeToDisplay(0x05, 0xff - 0b11110110);
	if (letter == 'S') writeToDisplay(0x05, 0xff - 0b11011010);
	if (letter == 'T') writeToDisplay(0x05, 0xff - 0b01100010);
	if (letter == 'U') writeToDisplay(0x05, 0xff - 0b01111100);
	if (letter == 'V') writeToDisplay(0x05, 0xff - 0b01011100);
	if (letter == 'W') writeToDisplay(0x05, 0xff - 0b00111010);
	if (letter == 'X') writeToDisplay(0x05, 0xff - 0b10010010);
	if (letter == 'Y') writeToDisplay(0x05, 0xff - 0b11010100);
	if (letter == 'Z') writeToDisplay(0x05, 0xff - 0b10110110);

	
	if (letter == 'a') writeToDisplay(0x05, 0xff - 0b10011000);
	if (letter == 'b') writeToDisplay(0x05, 0xff - 0b11111000);
	if (letter == 'c') writeToDisplay(0x05, 0xff - 0b10110000);
	if (letter == 'd') writeToDisplay(0x05, 0xff - 0b10111100);
	if (letter == 'e') writeToDisplay(0x05, 0xff - 0b00110000);
	if (letter == 'f') writeToDisplay(0x05, 0xff - 0b11100000);
	if (letter == 'g') writeToDisplay(0x05, 0xff - 0b10110010);
	if (letter == 'h') writeToDisplay(0x05, 0xff - 0b11101000);
	if (letter == 'i') writeToDisplay(0x05, 0xff - 0b00110010);
	if (letter == 'j') writeToDisplay(0x05, 0xff - 0b00011010);
	if (letter == 'k') writeToDisplay(0x05, 0xff - 0b11010010);
	if (letter == 'l') writeToDisplay(0x05, 0xff - 0b01100000);
	if (letter == 'm') writeToDisplay(0x05, 0xff - 0b10101010);
	if (letter == 'n') writeToDisplay(0x05, 0xff - 0b10101000);
	if (letter == 'o') writeToDisplay(0x05, 0xff - 0b10111000);
	if (letter == 'p') writeToDisplay(0x05, 0xff - 0b11100110);
	if (letter == 'q') writeToDisplay(0x05, 0xff - 0b10001110);
	if (letter == 'r') writeToDisplay(0x05, 0xff - 0b10100000);
	if (letter == 's') writeToDisplay(0x05, 0xff - 0b00011000);
	if (letter == 't') writeToDisplay(0x05, 0xff - 0b11110000);
	if (letter == 'u') writeToDisplay(0x05, 0xff - 0b00111000);
	if (letter == 'v') writeToDisplay(0x05, 0xff - 0b00011000);
	if (letter == 'w') writeToDisplay(0x05, 0xff - 0b01010100);
	if (letter == 'x') writeToDisplay(0x05, 0xff - 0b10010000);
	if (letter == 'y') writeToDisplay(0x05, 0xff - 0b11011100);
	if (letter == 'z') writeToDisplay(0x05, 0xff - 0b10010000);
	
}

void display_error(void) {
	display_letter_on_segment(-1 , 1);
	display_letter_on_segment('E', 2);

	// displayColon();
	
	display_letter_on_segment('r', 3);
	display_letter_on_segment('r', 4);
}

void display_credits(void) {

	display_letter_on_segment('b', 1);
	display_letter_on_segment('y', 2);

	displayColon();
	
	display_letter_on_segment('J', 3);
	display_letter_on_segment('L', 4);
}

void display_percent_sign(void) {
	displayNone();
	enableSegment(3);

	writeToDisplay(0x05, 0xff - 0b00011010);
	
	displayNone();
	enableSegment(4);
	
	writeToDisplay(0x05, 0xff - 0b01010010);
}

void display_percent(int percent) {
	int p = percent;
	
	if (p > 99) {
		p = 99;
	}
	if (p < 0) {
		p = 0;
	}

	int dig1 = p / 10;
	int dig2 = p - dig1 * 10;
	// dig2 is too noisy. Round to the nearest 5%.
	if (dig2 > 7) {
		dig1++;
		dig2 = 0;
	} else if (dig2 < 3) {
		dig2 = 0;
	} else {
		dig2 = 5;
	}

	if (dig1 >= 10) {
		dig1 = 9;
		dig2 = 9;
	}

	displayNumberOnSegment(dig1, 1);
	displayNumberOnSegment(dig2, 2);

	// displayColon();

	display_percent_sign();
}

void display_battery_voltage_mv(int mv) {
	if (mv > 0) {
		int dig1 = mv / 10000; // Should always be zero or something is going very wrong
		int dig2 = mv / 1000 - dig1 * 10;
		int dig3 = mv / 100 - dig1 * 100 - dig2 * 10;
		int dig4 = mv / 10 - dig1 * 1000 - dig2 * 100 - dig3 * 10;

		if (dig1 == 0) {
			dig1 = -1; // Don't show the highest hour time if using non-24 hr time.
		}
		displayTime(dig1, dig2, true, dig3, dig4);
	} else {
		display_error();
	}
}
