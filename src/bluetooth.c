/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic UART Bridge Service (NUS) sample
 */
// #include "uart_async_adapter.h"

#include <zephyr/types.h>
#include <zephyr/kernel.h>
// #include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <soc.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>

#include <bluetooth/services/nus.h>

#include <zephyr/settings/settings.h>

#include "main.h"
#include "bluetooth.h"

#define BT_LE_ADV_CONN_CUSTOM  BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE, 0x0b40, 0x0b40, NULL)
#define PRIORITY 7

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN	(sizeof(DEVICE_NAME) - 1)

#define UART_BUF_SIZE CONFIG_BT_NUS_UART_BUFFER_SIZE
#define UART_WAIT_FOR_BUF_DELAY K_MSEC(50)
#define UART_WAIT_FOR_RX CONFIG_BT_NUS_UART_RX_WAIT_TIME

static K_SEM_DEFINE(ble_init_ok, 0, 1);

static struct bt_conn *current_conn;
static struct bt_conn *auth_conn;


bool BLE_RECIEVED_FLAG;

struct uart_data_t {
	void *fifo_reserved;
	uint8_t data[UART_BUF_SIZE];
	uint16_t len;
};

// static struct uart_data_t *tx;
static K_FIFO_DEFINE(tx);
static bool DATA_REQUESTED = false;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

#if CONFIG_BT_NUS_UART_ASYNC_ADAPTER
UART_ASYNC_ADAPTER_INST_DEFINE(async_adapter);
#else
static const struct device *const async_adapter;
#endif

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void security_changed(struct bt_conn *conn, bt_security_t level,
			     enum bt_security_err err);
#endif

#if defined(CONFIG_BT_NUS_SECURITY_ENABLED)
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey);
static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey);
static void auth_cancel(struct bt_conn *conn);
static void pairing_complete(struct bt_conn *conn, bool bonded);
static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason);

static struct bt_conn_auth_cb conn_auth_callbacks = {
	.passkey_display = auth_passkey_display,
	.passkey_confirm = auth_passkey_confirm,
	.cancel = auth_cancel,
};

static struct bt_conn_auth_info_cb conn_auth_info_callbacks = {
	.pairing_complete = pairing_complete,
	.pairing_failed = pairing_failed
};
#else
static struct bt_conn_auth_cb conn_auth_callbacks;
static struct bt_conn_auth_info_cb conn_auth_info_callbacks;
#endif

static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data, uint16_t len);

static struct bt_nus_cb nus_cb = {
	.received = bt_receive_cb,
};

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void num_comp_reply(bool accept);
#endif

#if CONFIG_BT_NUS_UART_ASYNC_ADAPTER
UART_ASYNC_ADAPTER_INST_DEFINE(async_adapter);
#else
static const struct device *const async_adapter;
#endif

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected    = connected,
	.disconnected = disconnected,
#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
	.security_changed = security_changed,
#endif
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (err) {
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	current_conn = bt_conn_ref(conn);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (auth_conn) {
		bt_conn_unref(auth_conn);
		auth_conn = NULL;
	}

	if (current_conn) {
		bt_conn_unref(current_conn);
		current_conn = NULL;
	}
}

#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void security_changed(struct bt_conn *conn, bt_security_t level,
			     enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
}
#endif

#if defined(CONFIG_BT_NUS_SECURITY_ENABLED)
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
}

static void auth_passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];

	auth_conn = bt_conn_ref(conn);

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
}


static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
}


static void pairing_complete(struct bt_conn *conn, bool bonded)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
}


static void pairing_failed(struct bt_conn *conn, enum bt_security_err reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
}


#else
static struct bt_conn_auth_cb conn_auth_callbacks;
static struct bt_conn_auth_info_cb conn_auth_info_callbacks;
#endif

static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data,
			  uint16_t len)
{
	// These lines are important for some reason, I don't know why.
	// Removing them makes the device stop working
	char addr[BT_ADDR_LE_STR_LEN] = {0}; 
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));
	
	// if (BLE_RECIEVED_FLAG) return; // Make sure we're not in the middle of displaying something.
	// If we are, ignore this packet and wait.

	struct uart_data_t *ble_tx_buf = k_malloc(sizeof(*ble_tx_buf));

	for (uint16_t pos = 0; pos != len;) {
		// static struct uart_data_t *ble_tx_buf;

		if (!ble_tx_buf) {
			return;
		}

		// Keep the last byte of TX buffer for potential LF char.
		size_t tx_data_size = sizeof(ble_tx_buf->data) - 1;

		if ((len - pos) > tx_data_size) {
			ble_tx_buf->len = tx_data_size;
		} else {
			ble_tx_buf->len = (len - pos);
		}

		memcpy(ble_tx_buf->data, &data[pos], ble_tx_buf->len);

		pos += ble_tx_buf->len;

		// Append the LF character when the CR character triggered
		// transmission from the peer.
		 
		if ((pos == len) && (data[len - 1] == '\r')) {
			ble_tx_buf->data[ble_tx_buf->len] = '\n';
			ble_tx_buf->len++;
		}

		k_fifo_put(&tx, ble_tx_buf);
	}	
	k_free(ble_tx_buf);
	
	// k_fifo_put(&tx, buf);
	DATA_REQUESTED = true;
}


#ifdef CONFIG_BT_NUS_SECURITY_ENABLED
static void num_comp_reply(bool accept)
{
	if (accept) {
		bt_conn_auth_passkey_confirm(auth_conn);
	} else {
		bt_conn_auth_cancel(auth_conn);
	}

	bt_conn_unref(auth_conn);
	auth_conn = NULL;
}
#endif /* CONFIG_BT_NUS_SECURITY_ENABLED */

int BLE_init(void)
{
	int err = 0;
	BLE_RECIEVED_FLAG = false;

	if (IS_ENABLED(CONFIG_BT_NUS_SECURITY_ENABLED)) {
		err = bt_conn_auth_cb_register(&conn_auth_callbacks);
		if (err) {
			return 0;
		}

		err = bt_conn_auth_info_cb_register(&conn_auth_info_callbacks);
		if (err) {
			return 0;
		}
	}

	err = bt_enable(NULL);
	if (err) {
		return 0;
	}

	k_sem_give(&ble_init_ok);

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_nus_init(&nus_cb);
	if (err) {
		return 0;
	}

	// Advertises every 1.8 S
	err = bt_le_adv_start(BT_LE_ADV_CONN_CUSTOM, ad, ARRAY_SIZE(ad), sd,
			      ARRAY_SIZE(sd));
	
	// Advertises every 0.1 s
	// err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd,
	// 		      ARRAY_SIZE(sd));
	if (err) {
		return 0;
	}

	ble_write_thread();
}

void process_input(struct uart_data_t *input) {

	if ((input->data[0] == 'S') && (input->data[1] == 'T') && (input->data[2] == '=') 
			&& (input->data[5] == ':') && (input->data[8] == ':')){
		// ST=HH:MM:SS
		// Request to set the time (24 HR TIME)

		// Check input conditions
		int hr1 = (input->data[3]);
		int hr2 = (input->data[4]);
		int min1 = (input->data[6]);
		int min2 = (input->data[7]);
		int sec1 = (input->data[9]);
		int sec2 = (input->data[10]);

		hr1 -= 48;
		hr2 -= 48;
		int hr = hr1 * 10 + hr2;
		if (hr > 24) return;

		min1 -= 48;
		min2 -= 48;
		int min = min1 * 10 + min2;
		if (min > 60) return;

		sec1 -= 48;
		sec2 -= 48;
		int sec = sec1 * 10 + sec2;
		if (sec > 60) return;

		// Set time
		set_time((hr) * (60 * 60) + (min) * (60) + sec);
		BLE_RECIEVED_FLAG = true;
		return;
	} else if ((input->data[0] == 'S') && (input->data[1] == 'T') && (input->data[2] == '=') 
			&& (input->data[5] == ':')){
		// ST=HH:MM
		// Request to set the time (24 HR TIME)

		// Check input conditions
		int hr1 = (input->data[3]);
		int hr2 = (input->data[4]);
		int min1 = (input->data[6]);
		int min2 = (input->data[7]);
		int sec1 = (input->data[9]);
		int sec2 = (input->data[10]);

		hr1 -= 48;
		hr2 -= 48;
		int hr = hr1 * 10 + hr2;
		if (hr > 24) return;

		min1 -= 48;
		min2 -= 48;
		int min = min1 * 10 + min2;
		if (min > 60) return;

		// Set time
		set_time((hr) * (60 * 60) + (min) * (60));
		BLE_RECIEVED_FLAG = true;
		return;
	} else if ((input->data[0] == 'M') && (input->data[1] == 'T') && (input->data[2] == '=')) {
		// (MT=B)
		// Request to set Military time (24 HR TIME) Setting
		if (input->data[3] == '0'){
			// Military time = false
			set_military_time(false);
		} else {
			// Military time = true;
			set_military_time(true);
		}
		BLE_RECIEVED_FLAG = true;
		return;
	} else if ((input->data[0] == 'G') && (input->data[1] == 'B') && (input->data[2] == 'P')) {
		// Request to Get Battery Percentage (GBP)
		// For now, display percentage on the ring
		continue_showing_battery_percent();
		BLE_RECIEVED_FLAG = true;
		return;
	} else if ((input->data[0] == 'G') && (input->data[1] == 'B') && (input->data[2] == 'V')) {
		// Request to Get Battery Voltage (GBV)
		// For now, display voltage on the ring, using colon as period
		continue_showing_battery_voltage();
		BLE_RECIEVED_FLAG = true;
		return;
	} else if ((input->data[0] == 'S') && (input->data[1] == 'I')) {
		// Simulates IMU interrupt, to make developing less painful.
		simulate_IMU_interrupt();
		BLE_RECIEVED_FLAG = true;
		return;
	} else if ((input->data[0] == 'A') && (input->data[1] == 'O') && (input->data[2] == '=') && ((input->data[3] == '0') || (input->data[3] == '1'))) {
		bool req = false;
		if (input->data[3] == '1') req = true;
		set_always_on(req);
		BLE_RECIEVED_FLAG = true;
	} else if ((input->data[0] == 'D') && (input->data[1] == 'M') && (input->data[2] == 'W') && (input->data[3] == 'C') && (input->data[4] == '=') && ((input->data[5] >= '0') && (input->data[5] <= '9'))) {
		uint8_t req = (input->data[5] - '0');
		set_display_mode_while_charging(req);
		BLE_RECIEVED_FLAG = true;
	}
}


void ble_write_thread(void)
{
	/* Don't go any further until BLE is initialized */
	k_sem_take(&ble_init_ok, K_FOREVER);

	while(true) {
		while(!DATA_REQUESTED) {
			k_sleep(K_MSEC(10)); // This allows the system to actually do computations
		}

		BLE_recieved_data = k_fifo_get(&tx, K_SECONDS(1));

		if (bt_nus_send(NULL, BLE_recieved_data->data, BLE_recieved_data->len) == 0) {
			DATA_REQUESTED = false;
		}
		
		process_input(BLE_recieved_data);
	}
}
