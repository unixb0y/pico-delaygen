cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico-delaygen C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb")

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()

add_executable(${PROJECT_NAME}
    src/main.c
    src/oc.c
)

# pull in common dependencies
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_pio
    pico_unique_id
)

# Include header files
#target_include_directories(${PROJECT_NAME} 
#    PRIVATE 
#    "include"
#)

# Set custom USB name
target_compile_options(${PROJECT_NAME} PRIVATE "-DUSBD_PRODUCT=\"Delay Generator\"")

# disable uart output, enable usb output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# Create C header file with the name <pio program>.pio.h
pico_generate_pio_header(${PROJECT_NAME}  
    ${CMAKE_CURRENT_LIST_DIR}/src/square.pio
)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${PROJECT_NAME})

# clocking the flash so it keeps up
pico_define_boot_stage2(custom-boot ${PICO_DEFAULT_BOOT_STAGE2_FILE})
target_compile_definitions(custom-boot PRIVATE PICO_FLASH_SPI_CLKDIV=4)
pico_set_boot_stage2(${PROJECT_NAME} custom-boot)

