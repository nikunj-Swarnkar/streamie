# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

# Copy pico_sdk_import.cmake from <pico-sdk>/external/ into this folder first.
set(PICO_BOARD waveshare_rp2040_zero)
include(pico_sdk_import.cmake)

project(controller_xinput C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()

add_executable(controller_xinput main.c usb_xinput.c)

target_include_directories(controller_xinput PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(controller_xinput
    pico_stdlib
    hardware_spi
    hardware_pwm
    tinyusb_device)

pico_add_extra_outputs(controller_xinput)
