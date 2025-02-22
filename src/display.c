
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
	// k_msleep(1);
	k_usleep(SEGMENT_MIN_ON_TIME_US);

	// Ensure that this segment turns off at this time
	displayNone();
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

void display_error(uint8_t error_number) {
	if (error_number > 99) return; 

	uint8_t e1;
	uint8_t e2;
	
	e1 = error_number / 10;
	e2 = error_number % 10;

	display_word_chars('E', 'r', 'r', e1, e2);
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

	display_arb_all(num_to_segment(dig1), num_to_segment(dig2), 0x00 , 0b00011010, 0b01010010);
}

void display_battery_voltage_mv(int mv) {
	if (mv > 0) {
		int dig1 = mv / 1000;
		int dig2 = mv / 100 - dig1 * 10;
		int dig3 = mv / 10 - dig1 * 100 - dig2 * 10;
		int dig4 = mv / 1 - dig1 * 1000 - dig2 * 100 - dig3 * 10;

		if (mv > 9999) {
			dig1 = 9;
			dig2 = 9;
			dig3 = 9;
			dig4 = 9;
		}

		display_word_chars(dig1, '_', dig2, dig3, dig4);
	} else {
		display_error(99);
	}
}

uint8_t num_to_segment(uint8_t number){
	if ((number < 0) | (number > 9)) {
		return 0; // displays nothing
	} else {
		if (number == 0) return (0b01111110);
		if (number == 1) return (0b00001100);
		if (number == 2) return (0b10110110);
		if (number == 3) return (0b10011110);
		if (number == 4) return (0b11001100);
		if (number == 5) return (0b11011010);
		if (number == 6) return (0b11111010);
		if (number == 7) return (0b00001110);
		if (number == 8) return (0b11111110);
		if (number == 9) return (0b11011110);
	}

}

uint8_t char_to_segment(uint8_t character) {
	
	if ((character >= 0) & (character <= 9)) return num_to_segment(character); 
	if ((character >= '0') & (character <= '9')) return num_to_segment(character - '0'); 

	if ((character >= 'A') & (character <= 'Z')) {
		if (character == 'A') return (0b11101110);
		if (character == 'B') return (0b11111110);
		if (character == 'C') return (0b01110010);
		if (character == 'D') return (0b00111110);
		if (character == 'E') return (0b11110010);
		if (character == 'F') return (0b11100010);
		if (character == 'G') return (0b01111010);
		if (character == 'H') return (0b11101100);
		if (character == 'I') return (0b01100000);
		if (character == 'J') return (0b00111100);
		if (character == 'K') return (0b11101010);
		if (character == 'L') return (0b01110000);
		if (character == 'M') return (0b01010110);
		if (character == 'N') return (0b01101110);
		if (character == 'O') return (0b01111110);
		if (character == 'P') return (0b11100110);
		if (character == 'Q') return (0b11010110);
		if (character == 'R') return (0b11110110);
		if (character == 'S') return (0b11011010);
		if (character == 'T') return (0b01100010);
		if (character == 'U') return (0b01111100);
		if (character == 'V') return (0b01011100);
		if (character == 'W') return (0b00111010);
		if (character == 'X') return (0b10010010);
		if (character == 'Y') return (0b11010100);
		if (character == 'Z') return (0b10110110);
	} else if ((character >= 'a') & (character <= 'z')) {
		if (character == 'a') return (0b10011000);
		if (character == 'b') return (0b11111000);
		if (character == 'c') return (0b10110000);
		if (character == 'd') return (0b10111100);
		if (character == 'e') return (0b00110000);
		if (character == 'f') return (0b11100000);
		if (character == 'g') return (0b10110010);
		if (character == 'h') return (0b11101000);
		if (character == 'i') return (0b00110010);
		if (character == 'j') return (0b00011010);
		if (character == 'k') return (0b11010010);
		if (character == 'l') return (0b01100000);
		if (character == 'm') return (0b10101010);
		if (character == 'n') return (0b10101000);
		if (character == 'o') return (0b10111000);
		if (character == 'p') return (0b11100110);
		if (character == 'q') return (0b10001110);
		if (character == 'r') return (0b10100000);
		if (character == 's') return (0b00011000);
		if (character == 't') return (0b11110000);
		if (character == 'u') return (0b00111000);
		if (character == 'v') return (0b00011000);
		if (character == 'w') return (0b01010100);
		if (character == 'x') return (0b10010000);
		if (character == 'y') return (0b11011100);
		if (character == 'z') return (0b10010000);
	}
	if (character == '\"') return (0b01000100);
	if (character == '\'') return (0b01000000);
	if ((character == ':') | (character == ';') | (character == '=')) return (0b10010000);
	if (character == '-') return (0b10000000);
	if (character == '_') return (0b00010000);
	if (character == '(') return (0b01110010);
	if (character == ')') return (0b00011110);
	if (character == '*') return (0b11000110); // Supplements a degree symbol for temperature

	return (0x00); // Not a valid character
}

void display_arb_all(uint8_t s1, uint8_t s2, uint8_t s3, uint8_t s4, uint8_t s5){
	display_arb(1, s1);
	display_arb(2, s2);
	display_arb(3, s3);
	display_arb(4, s4);
	display_arb(5, s5);
}

// This is the slowest way possible to do this. If faster is needed, hard-code the values in.
void display_word_chars(char c1, char c2, char c3, char c4, char c5) {
	display_arb_all(char_to_segment(c1), char_to_segment(c2), char_to_segment(c3), char_to_segment(c4), char_to_segment(c5));
}

void display_word(char *word, int len) {
	if (len < 5) return; // Invalid word
	display_word_chars(word[0], word[1], word[2], word[3], word[4]);
}