cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(sensor_code)

target_include_directories(app PRIVATE inc)
target_sources(app PRIVATE src/main.c src/ble.c src/imu.c src/adc.c)
