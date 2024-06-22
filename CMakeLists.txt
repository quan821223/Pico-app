# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)


# 導入Pico SDK
# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# 設置項目名稱
project(host C CXX ASM)

# 設置標準
# Set name of project (as PROJECT_NAME) and C/C   standards
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 添加包含目錄
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 初始化Pico SDK
# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/tud_cdc_descript.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/command_handler.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/htu21df.c
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
	hardware_uart
	hardware_i2c
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/include
    $ENV{PICO_SDK_PATH}/include
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
