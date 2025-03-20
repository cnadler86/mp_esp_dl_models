#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "py/obj.h"
#include "py/runtime.h"

#include "dl_image_define.hpp"
#include <memory>

extern const mp_obj_type_t mp_face_detector_type;
extern const mp_obj_type_t mp_image_net_type;

#define MP_DEFINE_CONST_FUN_OBJ_0_CXX(obj_name, fun_name) \
    const mp_obj_fun_builtin_fixed_t obj_name = {.base = &mp_type_fun_builtin_0, .fun = {._0 = fun_name }}

#define MP_DEFINE_CONST_FUN_OBJ_1_CXX(obj_name, fun_name) \
    const mp_obj_fun_builtin_fixed_t obj_name = { .base = &mp_type_fun_builtin_1, .fun = {._1 = fun_name }}

#define MP_DEFINE_CONST_FUN_OBJ_2_CXX(obj_name, fun_name) \
    const mp_obj_fun_builtin_fixed_t obj_name = { .base = &mp_type_fun_builtin_2, .fun = {._2 = fun_name }}

#define MP_DEFINE_CONST_FUN_OBJ_3_CXX(obj_name, fun_name) \
    const mp_obj_fun_builtin_fixed_t obj_name = { .base = &mp_type_fun_builtin_3, .fun = {._3 = fun_name }}

#ifdef __cplusplus
}
#endif

void initialize_img(dl::image::img_t &img, int width, int height) {
    img.width = width;
    img.height = height;
    img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888;
    img.data = nullptr;
}

template <typename T>
T *get_and_validate_framebuffer(mp_obj_t self_in, mp_obj_t framebuffer_obj, dl::image::img_t &img) {
    // Cast self_in to the correct type
    T *self = static_cast<T *>(MP_OBJ_TO_PTR(self_in));

    // Validate the framebuffer
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(framebuffer_obj, &bufinfo, MP_BUFFER_READ);

    if (bufinfo.len != img.width * img.height * 3) {
        mp_raise_ValueError("Frame buffer size does not match the image size");
    }

    // Assign the buffer data to the image
    img.data = (uint8_t *)bufinfo.buf;

    return self;
}