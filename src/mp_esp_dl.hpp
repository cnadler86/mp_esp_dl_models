#pragma once

#ifdef __cplusplus
#include "dl_image_define.hpp"
#include <memory>
extern "C" {
#endif

#include "py/obj.h"
#include "py/runtime.h"

extern const mp_obj_type_t mp_face_detector_type;
extern const mp_obj_type_t mp_image_net_type;
extern const mp_obj_type_t mp_pedestrian_detector_type;
extern const mp_obj_type_t mp_face_recognizer_type;

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

# ifdef __cplusplus

namespace mp_esp_dl {
    void initialize_img(dl::image::img_t &img, int width, int height, dl::image::pix_type_t pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888);

    template <typename T>
    T *get_and_validate_framebuffer(mp_obj_t self_in, mp_obj_t framebuffer_obj) {
        // Cast self_in to the correct type
        T *self = static_cast<T *>(MP_OBJ_TO_PTR(self_in));

        // Validate the framebuffer
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(framebuffer_obj, &bufinfo, MP_BUFFER_READ);

        size_t expected_size = dl::image::get_img_byte_size(self->img);
        if (bufinfo.len != expected_size) {
            mp_raise_ValueError("Frame buffer size does not match the image size with the selected pixel format");
        }

        self->img.data = (uint8_t *)bufinfo.buf;

        return self;
    }

    template <typename T>
    void espdl_obj_property(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
        T *self = static_cast<T *>(MP_OBJ_TO_PTR(self_in));
        if (dest[0] == MP_OBJ_NULL) {
            switch (attr) {
                case MP_QSTR_width:
                    dest[0] = mp_obj_new_int(self->img.width);
                    break;
                case MP_QSTR_height:
                    dest[0] = mp_obj_new_int(self->img.height);
                    break;
                case MP_QSTR_pix_type:
                    dest[0] = mp_obj_new_int(self->img.pix_type);
                    break;
                default:
                    dest[1] = MP_OBJ_SENTINEL;
            }
        } else if (dest[1] != MP_OBJ_NULL) {
            switch (attr) {
                case MP_QSTR_width:
                    self->img.width = mp_obj_get_int(dest[1]);
                    break;
                case MP_QSTR_height:
                    self->img.height = mp_obj_get_int(dest[1]);
                    break;
                case MP_QSTR_pix_type:
                    self->img.pix_type = static_cast<dl::image::pix_type_t>(mp_obj_get_int(dest[1]));
                    break;
                default:
                    return;
            }
            dest[0] = MP_OBJ_NULL;
        }
    }
}

#endif