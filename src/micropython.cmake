include(${MICROPY_DIR}/py/py.cmake)

add_library(usermod_mp_esp_dl INTERFACE)

add_dependencies(usermod_mp_esp_dl human_face_detect)

target_include_directories(usermod_mp_esp_dl INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(usermod_mp_esp_dl INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/mp_esp_dl_module.c
    ${CMAKE_CURRENT_LIST_DIR}/esp_face_detector.cpp
)

if (MP_DL_IMAGENET_CLS_ENABLED)
    message(STATUS "Adding imagenet_cls model binding")
    target_compile_definitions(usermod_mp_esp_dl INTERFACE MP_DL_IMAGENET_CLS_ENABLED=1)
    add_dependencies(usermod_mp_esp_dl imagenet_cls)
    target_sources(usermod_mp_esp_dl INTERFACE 
        ${CMAKE_CURRENT_LIST_DIR}/esp_imagenet_cls.cpp
    )
endif()

if (MP_DL_PEDESTRIAN_DETECTOR_ENABLED)
    message(STATUS "Adding pedestrian_detect model binding")
    target_compile_definitions(usermod_mp_esp_dl INTERFACE MP_DL_PEDESTRIAN_DETECTOR_ENABLED=1)
    add_dependencies(usermod_mp_esp_dl pedestrian_detect)
    target_sources(usermod_mp_esp_dl INTERFACE 
        ${CMAKE_CURRENT_LIST_DIR}/esp_human_detector.cpp
    )
endif()

if (MP_DL_FACE_RECOGNITION_ENABLED)
    message(STATUS "Adding face_recognition model binding")
    target_compile_definitions(usermod_mp_esp_dl INTERFACE MP_DL_FACE_RECOGNITION_ENABLED=1)
    add_dependencies(usermod_mp_esp_dl human_face_recognition)
    target_compile_options(usermod INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-frtti>)
    target_sources(usermod_mp_esp_dl INTERFACE 
        ${CMAKE_CURRENT_LIST_DIR}/esp_face_recognition.cpp
        ${CMAKE_CURRENT_LIST_DIR}/lib/mp_esp_dl_recognition_database.cpp
        ${CMAKE_CURRENT_LIST_DIR}/lib/mp_esp_dl_human_face_recognition.cpp
        ${CMAKE_CURRENT_LIST_DIR}/lib/mpfile.c
    )
endif()

if (MP_DL_COCO_DETECTOR_ENABLED)
    message(STATUS "Adding coco_detect model binding")
    target_compile_definitions(usermod_mp_esp_dl INTERFACE MP_DL_COCO_DETECTOR_ENABLED=1)
    add_dependencies(usermod_mp_esp_dl coco_detect)
    target_sources(usermod_mp_esp_dl INTERFACE 
        ${CMAKE_CURRENT_LIST_DIR}/esp_coco_detector.cpp
    )
endif()

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

# Deactivate ROM text compression for module
target_compile_definitions(usermod_mp_esp_dl INTERFACE 
    MICROPY_ROM_TEXT_COMPRESSION=0
)

target_link_libraries(usermod INTERFACE usermod_mp_esp_dl)

micropy_gather_target_properties(usermod_mp_esp_dl)
