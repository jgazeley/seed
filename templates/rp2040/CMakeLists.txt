cmake_minimum_required(VERSION 3.13)

# Import the Raspberry Pi Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(${PROJECT_NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

# Create the executable
add_executable(${PROJECT_NAME}
    ${PROJECT_NAME}.c
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Create map/bin/hex files
pico_add_extra_outputs(${PROJECT_NAME})

# Link with pico_stdlib
target_link_libraries(${PROJECT_NAME} pico_stdlib)
