set(SDKCONFIG_DEFAULTS
    ${SDKCONFIG_DEFAULTS}
    ${MICROPY_PORT_DIR}/boards/sdkconfig.240mhz
    ${MICROPY_PORT_DIR}/boards/sdkconfig.spiram_oct
    boards/ESP32_GENERIC_S3/sdkconfig.flash_16m
)

list(APPEND MICROPY_DEF_BOARD
    MICROPY_HW_BOARD_NAME="Generic ESP32S3 module 16MB flash with Octal-SPIRAM"
)
