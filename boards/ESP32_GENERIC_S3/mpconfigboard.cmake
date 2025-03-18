set(IDF_TARGET esp32s3)

set(SDKCONFIG_DEFAULTS
    ${MICROPY_PORT_DIR}/boards/sdkconfig.base
    ${MICROPY_PORT_DIR}/boards/sdkconfig.usb
    ${MICROPY_PORT_DIR}/boards/sdkconfig.ble
    ${MICROPY_PORT_DIR}/boards/sdkconfig.spiram_sx
    ESP32_GENERIC_S3/sdkconfig.board
)

include($ENV{IDF_PATH}/tools/cmake/version.cmake)
set(IDF_VERSION "${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}.${IDF_VERSION_PATCH}")
message(STATUS "IDF version: ${IDF_VERSION}")
if (IDF_VERSION VERSION_GREATER_EQUAL "5.2.0")
    list(APPEND SDKCONFIG_DEFAULTS ${MICROPY_PORT_DIR}/boards/sdkconfig.idf52)
    message(STATUS "Adding the SDK config, final list is: ${SDKCONFIG_DEFAULTS}")
endif()
