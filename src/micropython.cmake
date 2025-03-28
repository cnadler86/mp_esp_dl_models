include(${MICROPY_DIR}/py/py.cmake)

add_library(usermod_mp_esp_dl INTERFACE)

add_dependencies(usermod_mp_esp_dl human_face_detect)

if (MP_DL_IMAGENET_CLS_ENABLED)
    target_compile_definitions(usermod_mp_esp_dl INTERFACE MP_DL_IMAGENET_CLS_ENABLED=1)
    add_dependencies(usermod_mp_esp_dl imagenet_cls)
endif()

if (MP_DL_PEDESTRISN_DETECTOR_ENABLED)
    target_compile_definitions(usermod_mp_esp_dl INTERFACE MP_DL_PEDESTRISN_DETECTOR_ENABLED=1)
    add_dependencies(usermod_mp_esp_dl pedestrian_detect)
endif()

if (MP_DL_FACE_RECOGNITION_ENABLED)
    target_compile_definitions(usermod_mp_esp_dl INTERFACE MP_DL_FACE_RECOGNITION_ENABLED=1)
    add_dependencies(usermod_mp_esp_dl human_face_recognition)
endif()

target_sources(usermod_mp_esp_dl INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/esp_face_detector.cpp
		${CMAKE_CURRENT_LIST_DIR}/esp_face_recognition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/esp_pedestrian_detector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/esp_imagenet_cls.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mp_esp_dl_module.c
    ${CMAKE_CURRENT_LIST_DIR}/mp_esp_dl.cpp
)

target_include_directories(usermod_mp_esp_dl INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

# Check if Camera is set or if Camera directory exists two levels up
if(DEFINED MP_CAMERA_DIR AND EXISTS "${MP_CAMERA_DIR}")
    message(STATUS "Using user-defined MP_CAMERA_DIR: ${MP_CAMERA_DIR}")
    set(MP_CAMERA_SRC "${MP_CAMERA_DIR}/src/micropython.cmake")
elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../micropython-camera-API")
    message(STATUS "Found micropython-camera-API directory at same level as mp_esp_dl module")
    set(MP_CAMERA_SRC "${CMAKE_CURRENT_LIST_DIR}/../../micropython-camera-API/src/micropython.cmake")
endif()

# Add MP_CAMERA_SRC cmake file to target_sources if it is defined
if(DEFINED MP_CAMERA_SRC AND EXISTS "${MP_CAMERA_SRC}")
    include(${MP_CAMERA_SRC})
else()
    message(WARNING "MP_CAMERA_SRC not found or not defined!")
    # Check if MP_JPEG_DIR is set or if mp_jpeg directory exists two levels up (Camera includes this normally)
    if(DEFINED MP_JPEG_DIR AND EXISTS "${MP_JPEG_DIR}")
        message(STATUS "Using user-defined MP_JPEG_DIR: ${MP_JPEG_DIR}")
        set(MP_JPEG_SRC "${MP_JPEG_DIR}/src/micropython.cmake")
    elseif(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../../mp_jpeg")
        message(STATUS "Found mp_jpeg directory at same level as mp_esp_dl module")
        set(MP_JPEG_SRC "${CMAKE_CURRENT_LIST_DIR}/../../mp_jpeg/src/micropython.cmake")
    endif()

    # Add MP_JPEG_SRC cmake file to target_sources if it is defined
    if(DEFINED MP_JPEG_SRC AND EXISTS "${MP_JPEG_SRC}")
        include(${MP_JPEG_SRC})
    else()
        message(WARNING "MP_JPEG_SRC not found or not defined!")
    endif()
endif()

target_link_libraries(usermod INTERFACE usermod_mp_esp_dl)

micropy_gather_target_properties(usermod_mp_esp_dl)
