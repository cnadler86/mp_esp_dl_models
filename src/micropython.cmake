include(${MICROPY_DIR}/py/py.cmake)

add_library(usermod_mp_esp_dl INTERFACE)

add_dependencies(usermod_mp_esp_dl human_face_detect)

target_include_directories(usermod_mp_esp_dl INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_sources(usermod_mp_esp_dl INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/mp_esp_dl_module.c
    ${CMAKE_CURRENT_LIST_DIR}/esp_face_detector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/esp_cat_detector.cpp
)

if (MP_DL_IMAGENET_CLS_ENABLED)
    message(STATUS "Adding imagenet_cls model binding")
    target_compile_definitions(usermod_mp_esp_dl INTERFACE MP_DL_IMAGENET_CLS_ENABLED=1)
    add_dependencies(usermod_mp_esp_dl imagenet_cls)
    target_sources(usermod_mp_esp_dl INTERFACE 
        ${CMAKE_CURRENT_LIST_DIR}/esp_imagenet_cls.cpp
    )
endif()


message(STATUS "Adding pedestrian_detect model binding")
target_compile_definitions(usermod_mp_esp_dl INTERFACE MP_DL_PEDESTRIAN_DETECTOR_ENABLED=1)
add_dependencies(usermod_mp_esp_dl pedestrian_detect)
target_sources(usermod_mp_esp_dl INTERFACE 
${CMAKE_CURRENT_LIST_DIR}/esp_human_detector.cpp
)

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

target_link_libraries(usermod INTERFACE usermod_mp_esp_dl)

micropy_gather_target_properties(usermod_mp_esp_dl)
