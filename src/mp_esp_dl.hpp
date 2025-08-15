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
extern const mp_obj_type_t mp_human_detector_type;
extern const mp_obj_type_t mp_face_recognizer_type;
extern const mp_obj_type_t mp_coco_detector_type;
extern const mp_obj_type_t mp_cat_detector_type;

#ifdef __cplusplus
}
#endif

# ifdef __cplusplus

namespace mp_esp_dl {

    template <typename TModel>
    struct MP_DetectorBase {
        mp_obj_base_t base;
        dl::image::img_t img;
        std::shared_ptr<TModel> model;
    };

    template <typename TDetector, typename TModel, typename... TArgs>
    TDetector* make_new(const mp_obj_type_t* type, int width, int height, 
                        dl::image::pix_type_t pix_type, TArgs&&... args) {
        TDetector* self = mp_obj_malloc_with_finaliser(TDetector, type);
        self->model = std::make_shared<TModel>(std::forward<TArgs>(args)...);
        if (!self->model) {
            mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to create model instance."));
        }
        self->img.width = width;
        self->img.height = height;
        self->img.pix_type = pix_type;
        self->img.data = nullptr;
        return self;
    }

    template <typename T>
    T *get_and_validate_framebuffer(mp_obj_t self_in, mp_obj_t framebuffer_obj) {
        // Cast self_in to the correct type
        T *self = static_cast<T *>(MP_OBJ_TO_PTR(self_in));

        // Validate the framebuffer
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(framebuffer_obj, &bufinfo, MP_BUFFER_READ);

        size_t expected_size = dl::image::get_img_byte_size(self->img);
        if (bufinfo.len != expected_size) {
            mp_raise_ValueError(MP_ERROR_TEXT("Frame buffer size does not match the image size with the selected pixel format."));
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
                case MP_QSTR_pixel_format:
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
                case MP_QSTR_pixel_format:
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