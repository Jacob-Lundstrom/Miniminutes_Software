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

#include <stdio.h>

#include <zephyr/logging/log.h>

#include "bluetooth.h"

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
	int err;
	char addr[BT_ADDR_LE_STR_LEN] = {0};

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, ARRAY_SIZE(addr));
	
	// struct uart_data_t *buf;

	for (uint16_t pos = 0; pos != len;) {
		struct uart_data_t *ble_tx_buf = k_malloc(sizeof(*ble_tx_buf));

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
	
	// k_fifo_put(&tx, buf);
	DATA_REQUESTED = true;
}

void error(void)
{
	while (true) {
		/* Spin for ever */
		k_sleep(K_MSEC(1000));
	}
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
		error();
	}

	k_sem_give(&ble_init_ok);

	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	err = bt_nus_init(&nus_cb);
	if (err) {
		return 0;
	}

	err = bt_le_adv_start(BT_LE_ADV_CONN_CUSTOM, ad, ARRAY_SIZE(ad), sd,
			      ARRAY_SIZE(sd));
	if (err) {
		return 0;
	}

	ble_write_thread();
}

void BLE_stop(void){
	bt_disable();
	bt_le_adv_stop();
}

void ble_write_thread(void)
{
	/* Don't go any further until BLE is initialized */
	k_sem_take(&ble_init_ok, K_FOREVER);

	for (;;) {
		while(!DATA_REQUESTED) k_sleep(K_MSEC(10));
		
		struct uart_data_t *buf = k_fifo_get(&tx,
						     K_FOREVER);

		if (bt_nus_send(NULL, buf->data, buf->len) == 0) {
			DATA_REQUESTED = false;
		}
	}
}
