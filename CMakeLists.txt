cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(PICO_SDK_FETCH_FROM_GIT 1)
include(pico_sdk_import.cmake)

project(joybus_pio CXX C ASM)

pico_sdk_init()

include_directories(include)

add_executable(
  GamecubeController
  src/main.cpp
  src/joybus.c
  src/GamecubeController.cpp
  src/usb_descriptors.cpp
)

pico_generate_pio_header(GamecubeController ${CMAKE_CURRENT_LIST_DIR}/src/joybus.pio)

target_link_libraries(GamecubeController pico_stdlib hardware_pio tinyusb_device tinyusb_board)

pico_enable_stdio_usb(GamecubeController 0)
pico_enable_stdio_uart(GamecubeController 1)

pico_add_extra_outputs(GamecubeController)