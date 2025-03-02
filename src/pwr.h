#ifndef PWR_H
#define PWR_H

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

#define BATTERY_ADC_SCALE_FACTOR (3.0/2.0)
#define BATTERY_MIN_VOLTAGE_MV 3500
#define BATTERY_MAX_VOLTAGE_MV 4200

#define GREEN_MIN_PERCENT 40
#define RED_MAX_PERCENT 60

int ADC_init(void);

uint32_t read_battery_voltage(void);
uint8_t get_battery_percentage(uint32_t battery_mv);

#endif