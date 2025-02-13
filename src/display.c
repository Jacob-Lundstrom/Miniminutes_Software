
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include "display.h"


int Display_init() {
	

	int ret = 0;
  	if (!gpio_is_ready_dt(&CA1)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CA1, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}
	
	if (!gpio_is_ready_dt(&CA2)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CA2, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	if (!gpio_is_ready_dt(&CA3)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CA3, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}
	
	if (!gpio_is_ready_dt(&CA4)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CA4, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	if (!gpio_is_ready_dt(&CA5)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CA5, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}
	
	if (!gpio_is_ready_dt(&CC1)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CC1, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	if (!gpio_is_ready_dt(&CC2)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CC2, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	if (!gpio_is_ready_dt(&CC3)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CC3, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	if (!gpio_is_ready_dt(&CC4)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CC4, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	if (!gpio_is_ready_dt(&CC5)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CC5, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	if (!gpio_is_ready_dt(&CC6)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CC6, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}

	if (!gpio_is_ready_dt(&CC7)) {
		return 0;
	}

	ret = gpio_pin_configure_dt(&CC7, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		return 0;
	}
}


void display_arb(int segment, uint8_t data) {
	displayNone();

	uint8_t out_states = 0xFF - data;
	gpio_pin_set_dt(&CC7, ((out_states >> 7) & 1));
	gpio_pin_set_dt(&CC6, ((out_states >> 6) & 1));
	gpio_pin_set_dt(&CC5, ((out_states >> 5) & 1));
	gpio_pin_set_dt(&CC4, ((out_states >> 4) & 1));
	gpio_pin_set_dt(&CC3, ((out_states >> 3) & 1));
	gpio_pin_set_dt(&CC2, ((out_states >> 2) & 1));
	gpio_pin_set_dt(&CC1, ((out_states >> 1) & 1));

	// last bit is not used.

	enableSegment(segment);
}

void enableSegment(int segment) {
  if (segment == 1) {
	gpio_pin_set_dt(&CA2, 0);
	gpio_pin_set_dt(&CA3, 0);
	gpio_pin_set_dt(&CA4, 0);
	gpio_pin_set_dt(&CA5, 0);

	gpio_pin_set_dt(&CA1, 1);
  } else if (segment == 2) {
	gpio_pin_set_dt(&CA1, 0);
	gpio_pin_set_dt(&CA3, 0);
	gpio_pin_set_dt(&CA4, 0);
	gpio_pin_set_dt(&CA5, 0);

	gpio_pin_set_dt(&CA2, 1);
  } else if (segment == 3) {
	gpio_pin_set_dt(&CA1, 0);
	gpio_pin_set_dt(&CA2, 0);
	gpio_pin_set_dt(&CA4, 0);
	gpio_pin_set_dt(&CA5, 0);

	gpio_pin_set_dt(&CA3, 1);
  } else if (segment == 4) {
	
	gpio_pin_set_dt(&CA1, 0);
	gpio_pin_set_dt(&CA2, 0);
	gpio_pin_set_dt(&CA3, 0);
	gpio_pin_set_dt(&CA5, 0);

	gpio_pin_set_dt(&CA4, 1);
  } else if (segment == 5) {
	
	gpio_pin_set_dt(&CA1, 0);
	gpio_pin_set_dt(&CA2, 0);
	gpio_pin_set_dt(&CA3, 0);
	gpio_pin_set_dt(&CA4, 0);

	gpio_pin_set_dt(&CA5, 1);
  } else {
    disableSegments();
  }
}

void disableSegments() {
	gpio_pin_set_dt(&CA1, 0);
	gpio_pin_set_dt(&CA2, 0);
	gpio_pin_set_dt(&CA3, 0);
	gpio_pin_set_dt(&CA4, 0);
	gpio_pin_set_dt(&CA5, 0);
}

void displayNone() {
	disableSegments();
}

void displayNumberOnSegment(uint8_t num, int segment) {
//   displayNone();
//   enableSegment(segment);

  // configureDisplay();
  if (num == 0) display_arb(segment, 0b01111110);
  if (num == 1) display_arb(segment, 0b00001100);
  if (num == 2) display_arb(segment, 0b10110110);
  if (num == 3) display_arb(segment, 0b10011110);
  if (num == 4) display_arb(segment, 0b11001100);
  if (num == 5) display_arb(segment, 0b11011010);
  if (num == 6) display_arb(segment, 0b11111010);
  if (num == 7) display_arb(segment, 0b00001110);
  if (num == 8) display_arb(segment, 0b11111110);
  if (num == 9) display_arb(segment, 0b11011110);
  if (num > 9) display_arb(segment, 0b00000000);
}

void displayColon(){
	display_arb(3, 0b10010000);
}

void displayTime(int num1, int num2, bool colon, int num4, int num5) {
	displayNumberOnSegment(num1, 1);
	displayNumberOnSegment(num2, 2);

	if (colon) {
		displayColon();
	}

	displayNumberOnSegment(num4, 4);
	displayNumberOnSegment(num5, 5);
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
	// enableSegment(segment);

	// I haven't double checked most of these.
	// Follows the sample given on https://en.wikipedia.org/wiki/Seven-segment_display_character_representations
	if (letter == 'A') display_arb(segment, 0b11101110);
	if (letter == 'B') display_arb(segment, 0b11111110);
	if (letter == 'C') display_arb(segment, 0b01110010);
	if (letter == 'D') display_arb(segment, 0b00111110);
	if (letter == 'E') display_arb(segment, 0b11110010);
	if (letter == 'F') display_arb(segment, 0b11100010);
	if (letter == 'G') display_arb(segment, 0b01111010);
	if (letter == 'H') display_arb(segment, 0b11101100);
	if (letter == 'I') display_arb(segment, 0b01100000);
	if (letter == 'J') display_arb(segment, 0b00111100);
	if (letter == 'K') display_arb(segment, 0b11101010);
	if (letter == 'L') display_arb(segment, 0b01110000);
	if (letter == 'M') display_arb(segment, 0b01010110);
	if (letter == 'N') display_arb(segment, 0b01101110);
	if (letter == 'O') display_arb(segment, 0b01111110);
	if (letter == 'P') display_arb(segment, 0b11100110);
	if (letter == 'Q') display_arb(segment, 0b11010110);
	if (letter == 'R') display_arb(segment, 0b11110110);
	if (letter == 'S') display_arb(segment, 0b11011010);
	if (letter == 'T') display_arb(segment, 0b01100010);
	if (letter == 'U') display_arb(segment, 0b01111100);
	if (letter == 'V') display_arb(segment, 0b01011100);
	if (letter == 'W') display_arb(segment, 0b00111010);
	if (letter == 'X') display_arb(segment, 0b10010010);
	if (letter == 'Y') display_arb(segment, 0b11010100);
	if (letter == 'Z') display_arb(segment, 0b10110110);

	
	if (letter == 'a') display_arb(segment, 0b10011000);
	if (letter == 'b') display_arb(segment, 0b11111000);
	if (letter == 'c') display_arb(segment, 0b10110000);
	if (letter == 'd') display_arb(segment, 0b10111100);
	if (letter == 'e') display_arb(segment, 0b00110000);
	if (letter == 'f') display_arb(segment, 0b11100000);
	if (letter == 'g') display_arb(segment, 0b10110010);
	if (letter == 'h') display_arb(segment, 0b11101000);
	if (letter == 'i') display_arb(segment, 0b00110010);
	if (letter == 'j') display_arb(segment, 0b00011010);
	if (letter == 'k') display_arb(segment, 0b11010010);
	if (letter == 'l') display_arb(segment, 0b01100000);
	if (letter == 'm') display_arb(segment, 0b10101010);
	if (letter == 'n') display_arb(segment, 0b10101000);
	if (letter == 'o') display_arb(segment, 0b10111000);
	if (letter == 'p') display_arb(segment, 0b11100110);
	if (letter == 'q') display_arb(segment, 0b10001110);
	if (letter == 'r') display_arb(segment, 0b10100000);
	if (letter == 's') display_arb(segment, 0b00011000);
	if (letter == 't') display_arb(segment, 0b11110000);
	if (letter == 'u') display_arb(segment, 0b00111000);
	if (letter == 'v') display_arb(segment, 0b00011000);
	if (letter == 'w') display_arb(segment, 0b01010100);
	if (letter == 'x') display_arb(segment, 0b10010000);
	if (letter == 'y') display_arb(segment, 0b11011100);
	if (letter == 'z') display_arb(segment, 0b10010000);
	
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

	display_arb(4, 0b00011010);
	
	displayNone();
	enableSegment(4);
	
	display_arb(5, 0b01010010);
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
