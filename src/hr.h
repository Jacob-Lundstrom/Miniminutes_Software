#include "main.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>

#define RED_CHANNEL 1
#define GREEN_CHANNEL 2
#define BLUE_CHANNEL 3
#define IR_CHANNEL 4

void setup_hr(void);

void illuminate_red(uint8_t current_mA);
void turn_off_red(void);
void reading_red(void);

void illuminate_green(uint8_t current_mA);
void turn_off_green(void);
void reading_green(void);

//Functions below here may not be used in final setup

void illuminate_blue(uint8_t current_mA);
void turn_off_blue(void);
void reading_blue(void);

void illuminate_IR(uint8_t current_mA);
void turn_off_IR(void);
void reading_IR(void);
