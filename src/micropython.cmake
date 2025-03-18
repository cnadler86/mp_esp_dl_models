include(${MICROPY_DIR}/py/py.cmake)

add_library(usermod_mp_esp_dl INTERFACE)

add_dependencies(usermod_mp_esp_dl human_face_detect)

target_sources(usermod_mp_esp_dl INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/esp_face_detector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/esp_image_net.cpp
    ${CMAKE_CURRENT_LIST_DIR}/esp_mp_dl_module.c
)

target_include_directories(usermod_mp_esp_dl INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Check if MP_JPEG_DIR is set or if mp_jpeg directory exists two levels up
if(DEFINED MP_JPEG_DIR AND EXISTS "${MP_JPEG_DIR}")
    message(STATUS "Using user-defined MP_JPEG_DIR: ${MP_JPEG_DIR}")
    set(MP_JPEG_SRC "${MP_JPEG_DIR}/src/micropython.cmake")
elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../mp_jpeg")
    message(STATUS "Found mp_jpeg directory two levels up")
    set(MP_JPEG_SRC "${CMAKE_CURRENT_LIST_DIR}/../../mp_jpeg/src/micropython.cmake")
endif()

# Add MP_JPEG_SRC cmake file to target_sources if it is defined
if(DEFINED MP_JPEG_SRC AND EXISTS "${MP_JPEG_SRC}")
    include(${MP_JPEG_SRC})
else()
    message(WARNING "MP_JPEG_SRC not found or not defined!")
endif()

target_link_libraries(usermod INTERFACE usermod_mp_esp_dl)

micropy_gather_target_properties(usermod_mp_esp_dl)
