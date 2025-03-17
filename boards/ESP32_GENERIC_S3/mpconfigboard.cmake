set(IDF_TARGET esp32s3)

message(STATUS "MICROPY_PORT_DIR is set to ${MICROPY_PORT_DIR}")

set(SDKCONFIG_DEFAULTS
    ${MICROPY_PORT_DIR}/boards/sdkconfig.base
    ${MICROPY_PORT_DIR}/boards/sdkconfig.usb
    ${MICROPY_PORT_DIR}/boards/sdkconfig.ble
    ${MICROPY_PORT_DIR}/boards/sdkconfig.spiram_sx
    boards/ESP32_GENERIC_S3/sdkconfig.board
)
