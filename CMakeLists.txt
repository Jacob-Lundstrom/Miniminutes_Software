# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky_1)

target_sources(app PRIVATE src/main.c src/display.c src/imu.c src/bluetooth.c src/pwr.c src/hr.c src/storage.c src/rtc.c src/display_als.c)

# target_sources(app PRIVATE src/main.c src/display.c src/imu.c src/bluetooth.c)
