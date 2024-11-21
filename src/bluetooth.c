#include "bluetooth.h"
#include <zephyr/bluetooth/bluetooth.h>

static void bt_ready(int err)
{
	if (err) {
		// printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	// printk("Bluetooth initialized\n");

	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		// printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	// printk("Beacon started\n");
}

int BLE_init() {
    int err;

	// printk("Starting Beacon Demo\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
	if (err) {
		// printk("Bluetooth init failed (err %d)\n", err);
        return 1;
	}
}