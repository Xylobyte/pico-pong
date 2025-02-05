cmake_minimum_required(VERSION 3.30)

set(PROJECT_NAME pong)
set(PICO_BOARD pico2_w)

set(PICO_PLATFORM rp2350-riscv)
set(PICO_TOOLCHAIN_PATH /Users/nantsa/pico/corev-openhw-gcc-macos)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)
include($ENV{PIMORONI_PICO_PATH}/pimoroni_pico_import.cmake)

project(${PROJECT_NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

pico_sdk_init()

add_executable(${PROJECT_NAME} main.cpp)

include(common/pimoroni_i2c)
include(common/pimoroni_bus)
include(libraries/pico_display_2/pico_display_2)
include(libraries/pico_graphics/pico_graphics)
include(drivers/st7789/st7789)
include(drivers/rgbled/rgbled)
include(drivers/button/button)

target_link_libraries(${PROJECT_NAME} pico_stdlib pico_rand hardware_spi hardware_pwm hardware_dma rgbled button pico_display_2 st7789 pico_graphics)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})
