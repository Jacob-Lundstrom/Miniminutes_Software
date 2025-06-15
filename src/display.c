
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include "display.h"

/**
 * \brief			Initializes the GPIO that are used to control the 7-segment digits.
 */
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

	return 0;
}


/**
 * \brief			Controls the enabling of the specified digit. Digits are enumerated starting with 1 referring to the furthest left digit.
 * \param			digit: Digit to enable
 */
void Display_enable_digit(int digit) {
	if (digit == 1) {
		gpio_pin_configure_dt(&CA1, GPIO_OUTPUT_ACTIVE);
	} else if (digit == 2) {
		gpio_pin_configure_dt(&CA2, GPIO_OUTPUT_ACTIVE);
	} else if (digit == 3) {
		gpio_pin_configure_dt(&CA3, GPIO_OUTPUT_ACTIVE);
	} else if (digit == 4) {
		gpio_pin_configure_dt(&CA4, GPIO_OUTPUT_ACTIVE);
	} else if (digit == 5) {
		gpio_pin_configure_dt(&CA5, GPIO_OUTPUT_ACTIVE);
	} else {
		Display_disable_all_digits();
	}
}

/**
 * \brief			Controls the disabling of the specified digit. Digits are enumerated starting with 1 referring to the furthest left digit.
 * \param			digit: Digit to disable
 */
void Display_disable_digit(int digit) {
	if (digit == 1) {
		gpio_pin_configure_dt(&CA1, GPIO_INPUT);
	} else if (digit == 2) {
		gpio_pin_configure_dt(&CA2, GPIO_INPUT);
	} else if (digit == 3) {
		gpio_pin_configure_dt(&CA3, GPIO_INPUT);
	} else if (digit == 4) {
		gpio_pin_configure_dt(&CA4, GPIO_INPUT);
	} else if (digit == 5) {
		gpio_pin_configure_dt(&CA5, GPIO_INPUT);
	} else {
		Display_disable_all_digits();
	}
}

/**
 * \brief			Controls the enabling of the specified digit with a low output. This should only be used to turn on indication LEDs.
 * \param			digit: Digit to enable with a low output.
 */
void Display_enable_digit_low(int digit) {
	if (digit == 1) {
	  gpio_pin_configure_dt(&CA1, GPIO_OUTPUT_INACTIVE);
	} else if (digit == 2) {
	  gpio_pin_configure_dt(&CA2, GPIO_OUTPUT_INACTIVE);
	} else if (digit == 3) {
	  gpio_pin_configure_dt(&CA3, GPIO_OUTPUT_INACTIVE);
	} else if (digit == 4) {
	  gpio_pin_configure_dt(&CA4, GPIO_OUTPUT_INACTIVE);
	} else if (digit == 5) {
	  gpio_pin_configure_dt(&CA5, GPIO_OUTPUT_INACTIVE);
	} else {
	  Display_disable_all_digits();
	}
}


/**
 * \brief			Changes all digits to inactive (High-Z).
 */
void Display_disable_all_digits() {
	gpio_pin_configure_dt(&CA1, GPIO_INPUT);
	gpio_pin_configure_dt(&CA2, GPIO_INPUT);
	gpio_pin_configure_dt(&CA3, GPIO_INPUT);
	gpio_pin_configure_dt(&CA4, GPIO_INPUT);
	gpio_pin_configure_dt(&CA5, GPIO_INPUT);
	
	// gpio_pin_set_dt(&CA5, 1);
	// gpio_pin_set_dt(&CA4, 1);
	// gpio_pin_set_dt(&CA3, 1);
	// gpio_pin_set_dt(&CA2, 1);
	// gpio_pin_set_dt(&CA1, 1);

	// Doing this massively reduces the quiescent current draw (by hundreds of uA).
	// I think this has something to do with GPIO leakage?
	gpio_pin_set_dt(&CC7, 1);
	gpio_pin_set_dt(&CC6, 1);
	gpio_pin_set_dt(&CC5, 1);
	gpio_pin_set_dt(&CC4, 1);
	gpio_pin_set_dt(&CC3, 1);
	gpio_pin_set_dt(&CC2, 1);
	gpio_pin_set_dt(&CC1, 1);
	
	// gpio_pin_configure_dt(&CC1, GPIO_INPUT);
	// gpio_pin_configure_dt(&CC2, GPIO_INPUT);
	// gpio_pin_configure_dt(&CC3, GPIO_INPUT);
	// gpio_pin_configure_dt(&CC4, GPIO_INPUT);
	// gpio_pin_configure_dt(&CC5, GPIO_INPUT);
	// gpio_pin_configure_dt(&CC6, GPIO_INPUT);
	// gpio_pin_configure_dt(&CC7, GPIO_INPUT);
}


/**
 * \brief			Helper function to turn off the display.
 */
void Display_display_none() {
	Display_disable_all_digits();
}


/**
 * \brief			Allows for the display of a number on a specified digit.
 * \param			num: Number to be displayed on a digit
 * \param			digit: Digit to show the requested number on
 */
void Display_display_number_on_digit(uint8_t num, int digit) {
  if (num == 0) Display_display_arbitrary(digit, 0b01111110);
  if (num == 1) Display_display_arbitrary(digit, 0b00001100);
  if (num == 2) Display_display_arbitrary(digit, 0b10110110);
  if (num == 3) Display_display_arbitrary(digit, 0b10011110);
  if (num == 4) Display_display_arbitrary(digit, 0b11001100);
  if (num == 5) Display_display_arbitrary(digit, 0b11011010);
  if (num == 6) Display_display_arbitrary(digit, 0b11111010);
  if (num == 7) Display_display_arbitrary(digit, 0b00001110);
  if (num == 8) Display_display_arbitrary(digit, 0b11111110);
  if (num == 9) Display_display_arbitrary(digit, 0b11011110);
  if (num > 9) Display_display_arbitrary(digit, 0b00000000);
}


/**
 * \brief			Displays a colon on digit 3. This is used in displaying the time.
 */
void Display_display_colon(){
	Display_display_arbitrary(3, Display_convert_character_to_segments(':'));
}


/**
 * \brief			Displays the time, with the ability to control what number is shown on each digit.
 * \param			numN: Number to display on digit N.
 * \param			colon: Controls whether or not the colon is on.
 * \param 			green_ind: Controls if the green indication LED will be illuminated
 * \param 			red_ind: Controls if the red indication LED will be illuminated
 */
void Display_display_time(int num1, int num2, bool colon, int num4, int num5, bool green_ind, bool red_ind) {
	Display_display_arb_all(Display_convert_number_to_segments(num1),
					Display_convert_number_to_segments(num2), 
					(colon*Display_convert_character_to_segments(':')),
					Display_convert_number_to_segments(num4), 
					Display_convert_number_to_segments(num5),
					green_ind,
					red_ind);
}


/**
 * \brief			Displays the current time, with the ability to toggle 24-hour time or 12-hour time.
 * \param			time_seconds: The time in seconds since midnight.
 * \param			time_24h: Controls whether the shown time is in 24-hour time (`true`) or 12-hour time (`false`)
 */
void Display_display_time_seconds(int time_seconds, bool time_24h, bool show_colon) {
	
	int hr = time_seconds / (60*60) ;
	int min = time_seconds / (60) - 60 * hr;
	
	if (!time_24h) {
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

	if (!time_24h) {
		if (dig1 == 0) {
			dig1 = -1; // Don't show the highest hour time if using non-24 hr time.
		}
	}
	Display_display_time(dig1, dig2, show_colon, dig3, dig4, false, false);
}


/**
 * \brief			Quick function that dispays a specified character on the desired digit.
 * \param			letter: Character to be displayed
 * \param			digit: The digit to display the requested character
 */
void display_letter_on_segment(char letter, int digit) {
	Display_display_arbitrary(digit, Display_convert_character_to_segments(letter));
}


/**
 * \brief			Displays the provided error code on the display.
 * \param			error_number: The corresponding error number to be displayed.
 */
void Display_display_error(uint8_t error_number) {
	if (error_number > 99) return; 

	uint8_t e1;
	uint8_t e2;
	
	e1 = error_number / 10;
	e2 = error_number % 10;

	Display_display_word_chars('E', 'r', 'r', e1, e2, 0, 1);
}


/**
 * \brief			Displays credits.
 */
void Display_display_credits(void) {
	Display_display_word("by:JL", 5, 1, 1);
}


/**
 * \brief			Displays the battery percentage on the displays.
 * \param			percent: The battery's percent charge.
 */
void Display_display_percent(int percent) {
	int p = percent;

	uint16_t T_flash_ms = 250; // ms
	bool g = (p >= GREEN_MIN_PERCENT);
	bool r = (p <= RED_MAX_PERCENT) - (!is_charging) * (p <= RED_FLASHING_MAX_PERCENT) * ((k_uptime_get() % (T_flash_ms)) / (T_flash_ms / 2));


	if (p > 99) {
		Display_display_arb_all(Display_convert_number_to_segments(1), Display_convert_number_to_segments(0), Display_convert_number_to_segments(0), 0b00011010, 0b01010010, 1, 0);
		return;
	}
	
	if (p < 0) {
		Display_display_arb_all(Display_convert_number_to_segments(0), 0x00, 0x00, 0b00011010, 0b01010010, 0, 1);
		return;
	}

	int dig1 = p / 10;
	int dig2 = p - dig1 * 10;
	
	if (p < 10) {
		Display_display_arb_all(Display_convert_number_to_segments(dig2), 0x00, 0x00, 0b00011010, 0b01010010, g, r);
		return;
	}

	Display_display_arb_all(Display_convert_number_to_segments(dig1), Display_convert_number_to_segments(dig2), 0x00 , 0b00011010, 0b01010010, g, r);
}


/**
 * \brief			Displays the Battery voltage on the display.
 * \param			mv: The battery voltage, in millivolts.
 */
void Display_display_battery_voltage_mv(int mv) {
	int p = get_battery_percentage(mv);
	if (mv > 0) {
		int dig1 = mv / 1000;
		int dig2 = mv / 100 - dig1 * 10;
		int dig3 = mv / 10 - dig1 * 100 - dig2 * 10;
		int dig4 = mv / 1 - dig1 * 1000 - dig2 * 100 - dig3 * 10;
		
		bool g = 0;
		bool r = 0;

		g = (p >= GREEN_MIN_PERCENT);
		r = (p <= RED_MAX_PERCENT);

		if (mv > 9999) {
			dig1 = 9;
			dig2 = 9;
			dig3 = 9;
			dig4 = 9;
		}

		Display_display_word_chars(dig1, '_', dig2, dig3, dig4, g, r);
	} else {
		Display_display_error(99);
	}
}


/**
 * \brief			A Helper function that returns an 8-bit number that corresponds to the segments which should be illuminated to display the requested number
 * \param			number: input number to convert to LED data. Valid numbers are 0-9.
 * \return			Data corresponding to the correct segments to illuminate for the number to be displayed.
 */
uint8_t Display_convert_number_to_segments(uint8_t number){
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


/**
 * \brief			A Helper function that returns an 8-bit number that corresponds to the segments which should be illuminated to display the requested character
 * \param			character: input character to convert to LED data
 * \return			Data corresponding to the correct segments to illuminate for the character to be displayed.
 */
uint8_t Display_convert_character_to_segments(uint8_t character) {
	
	// I haven't double checked most of these.
	// Follows the sample given on https://en.wikipedia.org/wiki/Seven-segment_display_character_representations
	
	if ((character >= 0) & (character <= 9)) return Display_convert_number_to_segments(character); 
	if ((character >= '0') & (character <= '9')) return Display_convert_number_to_segments(character - '0'); 

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
	} 
	else if ((character >= 'a') & (character <= 'z')) {
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
	
	// Special Characters
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


/**
 * \brief			Allows the display of arbitrary segment configurations on a given 7-segment digit.
 * \param			digit: Digit which will have the requested character displayed on.
 * \param			data: Data corresponding to the configuration to be displayed. Bits 7-1 correspond to segments G-A, respectively. Bit 0 is used to enable an indication LED, if it is associated with the display digit. Otherwise, bit 0 does nothing.
 */
void Display_display_arbitrary(int digit, uint8_t data) {

	uint8_t out_states = 0xFF - data;
	gpio_pin_set_dt(&CC7, ((out_states >> 7) & 1));
	gpio_pin_set_dt(&CC6, ((out_states >> 6) & 1));
	gpio_pin_set_dt(&CC5, ((out_states >> 5) & 1));
	gpio_pin_set_dt(&CC4, ((out_states >> 4) & 1));
	gpio_pin_set_dt(&CC3, ((out_states >> 3) & 1));
	gpio_pin_set_dt(&CC2, ((out_states >> 2) & 1));
	gpio_pin_set_dt(&CC1, ((out_states >> 1) & 1));

	if (digit == 1) {
		// Check if we should turn on the corresponding indication LEDs
		if (data & 1) {
			Display_enable_digit_low(2); // Red
		}
	}

	if (digit == 2) {
		// Check if we should turn on the corresponding indication LEDs
		if (data & 1) {
			Display_enable_digit_low(1); // Green
		}
	}

	// These need to be modified to be variable duty (variable brightness)
	Display_enable_digit(digit);
	k_usleep(SEGMENT_MIN_ON_TIME_US);
	// Display_disable_digit(digit);
	Display_disable_all_digits();
	k_usleep(SEGMENT_MIN_OFF_TIME_US);
}


/**
 * \brief           Allows the display of arbitrary digit configurations on all digits
 * \param[in]       sN: Data corresponding to the configuration to be displayed on digit N. Bits 7-1 correspond to segments G-A, respectively. Bit 0 does nothing.
 * \param[in]       green_ind: Controls if the green indication LED will be illuminated
 * \param[in]       red_ind: Controls if the red indication LED will be illuminated
 */
void Display_display_arb_all(uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, bool green_ind, bool red_ind){
	Display_display_arbitrary(1, d1 & 0xFE);
	Display_display_arbitrary(1, red_ind);
	
	Display_display_arbitrary(2, d2 & 0xFE);
	Display_display_arbitrary(2, green_ind);
	
	Display_display_arbitrary(3, d3);
	
	Display_display_arbitrary(4, d4);
	
	Display_display_arbitrary(5, d5);
}


/**
 * \brief           Displays the given characters on their corresponding digit.
 * \param[in]       cN: Character to be displayed on display N.
 * \param[in]       green_ind: Controls if the green indication LED will be illuminated
 * \param[in]       red_ind: Controls if the red indication LED will be illuminated
 */
void Display_display_word_chars(char c1, char c2, char c3, char c4, char c5, bool green_ind, bool red_ind) {
	Display_display_arb_all(Display_convert_character_to_segments(c1), Display_convert_character_to_segments(c2), Display_convert_character_to_segments(c3), Display_convert_character_to_segments(c4), Display_convert_character_to_segments(c5), green_ind, red_ind);
}


/**
 * \brief 			Displays the given word on all of the digits in the display.
 * \param 			word: Word to be displayed on the displays. Maximumlength of 5 or more characters.
 * \param 			len: Length of the word to be displayed
 * \param 			green_ind: Controls if the green indication LED will be illuminated
 * \param 			red_ind: Controls if the red indication LED will be illuminated
 */
void Display_display_word(char *word, int len, bool green_ind, bool red_ind) {
	if (len > 5) return; // Invalid word
	uint8_t current_digit = 5 - len + 1;
	uint8_t* characters[5] = {'?','?','?','?','?'};
	uint8_t count = 0;
	while (current_digit < 6) {
		characters[current_digit++ - 1] = word[count++]; 
	}
	Display_display_word_chars(characters[0], characters[1], characters[2], characters[3], characters[4], green_ind, red_ind);
}

/**
 * \brief 			Displays the given integer on the display.
 * \param 			integer: Requested integer to display.
 * \param 			green_ind: Controls if the green indication LED will be illuminated
 * \param 			red_ind: Controls if the red indication LED will be illuminated
 */
void Display_display_integer(uint16_t integer, bool green_ind, bool red_ind) {
	if (integer >= 0) {
		int dig1 = integer / 10000;
		int dig2 = integer / 1000 - dig1 * 10;
		int dig3 = integer / 100 - dig1 * 100 - dig2 * 10;
		int dig4 = integer / 10 - dig1 * 1000 - dig2 * 100 - dig3 * 10;
		int dig5 = integer / 1 - dig1 * 10000 - dig2 * 1000 - dig3 * 100 - dig4 * 10;
		Display_display_word_chars(dig1, dig2, dig3, dig4, dig5, green_ind, red_ind);
	} else {
		// Display_display_error(99);
		return;
	}
}