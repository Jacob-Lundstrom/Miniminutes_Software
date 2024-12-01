#include <zephyr/kernel.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/logging/log.h>

#include "main.h"

// LOG_MODULE_REGISTER(main);

#define WDT_DEV_NAME        DT_NODELABEL(wdt)
#define WDT_TIMEOUT_MS      1000  // Watchdog timeout in milliseconds

static const struct device *wdt_dev;
static int wdt_channel_id;

void feed_watchdog(void) {
    static struct wdt_timeout_cfg wdt_config = {
        .window = {
            .min = 0,
            .max = WDT_TIMEOUT_MS,
        },
        .callback = reset_BLE,  // Null means system reset on timeout
    };

    if (wdt_channel_id < 0) {
        return;
    }

    wdt_feed(wdt_dev, wdt_channel_id);
}

void watchdog_init(void) {
     // Initialize watchdog
    wdt_dev = DEVICE_DT_GET(WDT_DEV_NAME);
    if (!device_is_ready(wdt_dev)) {
        // LOG_ERR("Watchdog device not ready");
        return;
    }

    struct wdt_timeout_cfg wdt_config = {
        .window.min = 0U,
        .window.max = WDT_TIMEOUT_MS,
        .callback = reset_BLE // NULL means system reset
    };

    wdt_channel_id = wdt_install_timeout(wdt_dev, &wdt_config);
    if (wdt_channel_id < 0) {
        // LOG_ERR("Failed to install watchdog timeout");
        return;
    }

    if (wdt_setup(wdt_dev, 0) < 0) {
        // LOG_ERR("Failed to setup watchdog");
        return;
    }
}