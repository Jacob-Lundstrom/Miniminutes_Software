#include "pwr.h"


#define BATTERY_ADC_SCALE_FACTOR (3.0/2.0)

int err;
uint32_t count = 0;
uint16_t buf;
struct adc_sequence sequence = {
	.buffer = &buf,
	/* buffer size in bytes, not number of samples */
	.buffer_size = sizeof(buf),
};

int ADC_init(void)
{
	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!adc_is_ready_dt(&adc_channels[i])) {
			// printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
			return 0;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			// printk("Could not setup channel #%d (%d)\n", i, err);
			return 0;
		}
	}
}

uint32_t read_battery_voltage(){
	uint32_t avg = 0;
	int rdgs = 100;
	// printk("ADC reading[%u]:\n", count++);
	for(int k = 0; k < rdgs; k++) {
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		int32_t val_mv;

		printk("- %s, channel %d: ",
				adc_channels[i].dev->name,
				adc_channels[i].channel_id);

		(void)adc_sequence_init_dt(&adc_channels[i], &sequence);

		err = adc_read_dt(&adc_channels[i], &sequence);
		if (err < 0) {
			// printk("Could not read (%d)\n", err);
			continue;
		}

		/*
			* If using differential mode, the 16 bit value
			* in the ADC sample buffer should be a signed 2's
			* complement value.
			*/
		if (adc_channels[i].channel_cfg.differential) {
			val_mv = (int32_t)((int16_t)buf);
		} else {
			val_mv = (int32_t)buf;
		}
		// printk("%"PRId32, val_mv);
		err = adc_raw_to_millivolts_dt(&adc_channels[i],
							&val_mv);
		/* conversion to mV may not be supported, skip if not */
		if (err < 0) {
			// printk(" (value in mV not available)\n");
		} else {
			avg += val_mv;
		}
	}
	}
	return (avg / rdgs) * BATTERY_ADC_SCALE_FACTOR;
}