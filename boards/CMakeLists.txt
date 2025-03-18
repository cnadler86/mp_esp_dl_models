# Top-level cmake file for building MicroPython on ESP32.
#
# Note for maintainers: Where possible, functionality should be put into
# esp32_common.cmake not this file. This is because this CMakeLists.txt file
# needs to be duplicated for out-of-tree builds, and can easily get out of date.
cmake_minimum_required(VERSION 3.12)

# Set the location of MicroPython, the esp32 port, and the board directory.
if(NOT MICROPY_BOARD)
    set(MICROPY_BOARD ESP32_GENERIC_S3)
endif()
get_filename_component(PROJECT_DIR ".." ABSOLUTE)
get_filename_component(USER_C_MODULES "../src/micropython.cmake" ABSOLUTE)
if(NOT MICROPY_DIR)
    get_filename_component(MICROPY_DIR "../../micropython" ABSOLUTE)
endif()
set(ENV{EV_MICROPY_DIR} "${MICROPY_DIR}")
set(MICROPY_PORT_DIR ${MICROPY_DIR}/ports/esp32)
if(NOT MICROPY_BOARD_DIR)
    set(MICROPY_BOARD_DIR ${CMAKE_CURRENT_LIST_DIR}/${MICROPY_BOARD})
endif()

message(STATUS "MICROPY_DIR is set to ${MICROPY_DIR}")
message(STATUS "MICROPY_PORT_DIR is set to ${MICROPY_PORT_DIR}")
message(STATUS "MICROPY_BOARD_DIR is set to ${MICROPY_BOARD_DIR}")

if(NOT EXISTS ${MICROPY_BOARD_DIR}/mpconfigboard.cmake)
    message(FATAL_ERROR "Invalid MICROPY_BOARD specified: ${MICROPY_BOARD}")
endif()

# If a board variant is specified, check that it exists.
if(MICROPY_BOARD_VARIANT)
    if(NOT EXISTS ${MICROPY_BOARD_DIR}/mpconfigvariant_${MICROPY_BOARD_VARIANT}.cmake)
        message(FATAL_ERROR "Invalid MICROPY_BOARD_VARIANT specified: ${MICROPY_BOARD_VARIANT}")
    endif()
endif()

# Define the output sdkconfig so it goes in the build directory.
set(SDKCONFIG ${CMAKE_BINARY_DIR}/sdkconfig)

# Save the manifest file set from the cmake command line.
set(MICROPY_USER_FROZEN_MANIFEST ${MICROPY_FROZEN_MANIFEST})

# Include board config; this is expected to set (among other options):
# - SDKCONFIG_DEFAULTS
# - IDF_TARGET
include(${MICROPY_BOARD_DIR}/mpconfigboard.cmake)
if(NOT MICROPY_BOARD_VARIANT)
    include(${MICROPY_BOARD_DIR}/mpconfigvariant.cmake OPTIONAL)
else()
    include(${MICROPY_BOARD_DIR}/mpconfigvariant_${MICROPY_BOARD_VARIANT}.cmake)
endif()

# Set the frozen manifest file. Note if MICROPY_FROZEN_MANIFEST is set from the cmake
# command line, then it will override the default and any manifest set by the board.
if (MICROPY_USER_FROZEN_MANIFEST)
    set(MICROPY_FROZEN_MANIFEST ${MICROPY_USER_FROZEN_MANIFEST})
elseif (NOT MICROPY_FROZEN_MANIFEST)
    set(MICROPY_FROZEN_MANIFEST ${CMAKE_CURRENT_LIST_DIR}/boards/manifest.py)
endif()

# Concatenate all sdkconfig files into a combined one for the IDF to use.
file(WRITE ${CMAKE_BINARY_DIR}/sdkconfig.combined.in "")
foreach(SDKCONFIG_DEFAULT ${SDKCONFIG_DEFAULTS})
    file(READ ${SDKCONFIG_DEFAULT} CONTENTS)
    file(APPEND ${CMAKE_BINARY_DIR}/sdkconfig.combined.in "${CONTENTS}")
endforeach()
configure_file(${CMAKE_BINARY_DIR}/sdkconfig.combined.in ${CMAKE_BINARY_DIR}/sdkconfig.combined COPYONLY)
set(SDKCONFIG_DEFAULTS ${CMAKE_BINARY_DIR}/sdkconfig.combined)

# set(MICROPY_USER_LDFRAGMENTS ${MICROPY_PORT_DIR}/main_${IDF_TARGET}/linker.lf)

# Include main IDF cmake file.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# Define the project.
project(micropython)
