set(MICROPY_DIR $ENV{EV_MICROPY_DIR})
get_filename_component(MICROPY_PORT_DIR ${MICROPY_DIR}/ports/esp32 ABSOLUTE)

set(PROJECT_DIR ${MICROPY_PORT_DIR})
if(EXISTS ${MICROPY_PORT_DIR}/main_esp32s3/CMakeLists.txt)
    include(${MICROPY_PORT_DIR}/main_esp32s3/CMakeLists.txt)
elseif(EXISTS ${MICROPY_PORT_DIR}/main/CMakeLists.txt)
    include(${MICROPY_PORT_DIR}/main/CMakeLists.txt)
else()
    message(FATAL_ERROR "main CMakeLists.txt not found in ${MICROPY_PORT_DIR}")
endif()