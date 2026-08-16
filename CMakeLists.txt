# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Set name of project (as PROJECT_NAME) and C/C   standards
project(host C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

if(NOT PICO_SDK_VERSION_STRING STREQUAL "2.1.1")
    message(FATAL_ERROR
        "This project requires Pico SDK 2.1.1; found ${PICO_SDK_VERSION_STRING}")
endif()

if(PICO_BOARD STREQUAL "pico")
    set(APP_ACTIVE_BOARD_ID 0)
elseif(PICO_BOARD STREQUAL "pico_w")
    set(APP_ACTIVE_BOARD_ID 1)
elseif(PICO_BOARD STREQUAL "waveshare_rp2040_zero")
    set(APP_ACTIVE_BOARD_ID 2)
elseif(PICO_BOARD STREQUAL "pico2")
    set(APP_ACTIVE_BOARD_ID 3)
else()
    message(FATAL_ERROR
        "Unsupported PICO_BOARD '${PICO_BOARD}'. Supported boards: "
        "pico, pico_w, waveshare_rp2040_zero, pico2")
endif()

message(STATUS "Application board profile: ${PICO_BOARD} (id=${APP_ACTIVE_BOARD_ID})")

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/app_protocol.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application/protocol_service.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board/board_profile.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/config_journal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/configuration_service.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/crc32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config/runtime_config.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/pico/pico_app_io.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/pico/pico_config_storage.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/pico/pico_status_indicator.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol/protocol_stream_parser.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/application
    ${CMAKE_CURRENT_SOURCE_DIR}/src/board
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config
    ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/pico
    ${CMAKE_CURRENT_SOURCE_DIR}/src/protocol
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    APP_ACTIVE_BOARD_ID=${APP_ACTIVE_BOARD_ID}
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_flash
    hardware_sync
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
