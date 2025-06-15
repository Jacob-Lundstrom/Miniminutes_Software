/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic UART Bridge Service (NUS) sample
 */


#define BLE_STACKSIZE CONFIG_BT_NUS_THREAD_STACK_SIZE

static struct uart_data_t *BLE_recieved_data;

int BLE_init(void);

void ble_write_thread(void);

void process_input(struct uart_data_t *input);
