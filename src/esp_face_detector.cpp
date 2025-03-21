#include "mp_esp_dl.hpp"
#include "freertos/idf_additions.h"
#include "human_face_detect.hpp"
#include <memory>

namespace mp_esp_dl::detector {

// Object
struct MP_FaceDetector {
    mp_obj_base_t base;
    std::shared_ptr<HumanFaceDetect> model = nullptr;
    dl::image::img_t img;
    bool return_features;
};

// Constructor
static mp_obj_t face_detector_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    enum { ARG_img_width, ARG_img_height, ARG_return_features };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width, MP_ARG_INT, {.u_int = 320} },
        { MP_QSTR_height, MP_ARG_INT, {.u_int = 240} },
        { MP_QSTR_features, MP_ARG_BOOL, {.u_bool = false} },
    };

    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    MP_FaceDetector *self = mp_obj_malloc_with_finaliser(MP_FaceDetector, &mp_face_detector_type);
    self->model = std::make_shared<HumanFaceDetect>();
    
    if (!self->model) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to create model instance");
    }

    mp_esp_dl::initialize_img(self->img, parsed_args[ARG_img_width].u_int, parsed_args[ARG_img_height].u_int);
    // self->img.width = parsed_args[ARG_img_width].u_int;
    // self->img.height = parsed_args[ARG_img_height].u_int;
    // self->img.pix_type = dl::image::DL_IMAGE_PIX_TYPE_RGB888;
    // self->img.data = nullptr;

    self->return_features = parsed_args[ARG_return_features].u_bool;

    return MP_OBJ_FROM_PTR(self);
}

// Destructor
static mp_obj_t face_detector_del(mp_obj_t self_in) {
    MP_FaceDetector *self = static_cast<MP_FaceDetector *>(MP_OBJ_TO_PTR(self_in));
    self->model = nullptr;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1_CXX(face_detector_del_obj, face_detector_del);

// Detect method
static mp_obj_t face_detector_detect(mp_obj_t self_in, mp_obj_t framebuffer_obj) {
    MP_FaceDetector *self = static_cast<MP_FaceDetector *>(MP_OBJ_TO_PTR(self_in));

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(framebuffer_obj, &bufinfo, MP_BUFFER_READ);

    if (bufinfo.len != self->img.width * self->img.height * 3) {
        mp_raise_ValueError("Frame buffer size does not match the image size");
    } else {
        self->img.data = (uint8_t *)bufinfo.buf;
    }

    auto &detect_results = self->model->run(self->img);

    if (detect_results.size() == 0) {
        return mp_const_none;
    }

    mp_obj_t list = mp_obj_new_list(0, NULL);
    for (const auto &res : detect_results) {
        mp_obj_t tuple[4];
        for (int i = 0; i < 4; ++i) {
            tuple[i] = mp_obj_new_int(res.box[i]);
        }
        mp_obj_list_append(list, mp_obj_new_tuple(4, tuple));
    
        if (self->return_features) {
            mp_obj_t features[10];
            for (int i = 0; i < 10; ++i) {
                features[i] = mp_obj_new_int(res.keypoint[i]);
            }
            mp_obj_list_append(list, mp_obj_new_tuple(10, features));
        }
    }
    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_2_CXX(face_detector_detect_obj, face_detector_detect);

// Local dict
static const mp_rom_map_elem_t face_detector_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_detect), MP_ROM_PTR(&face_detector_detect_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&face_detector_del_obj) },
};
static MP_DEFINE_CONST_DICT(face_detector_locals_dict, face_detector_locals_dict_table);

// Print
static void print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    mp_printf(print, "Face detector object");
}

} //namespace

// Type
MP_DEFINE_CONST_OBJ_TYPE(
    mp_face_detector_type,
    MP_QSTR_FaceDetector,
    MP_TYPE_FLAG_NONE,
    make_new, (const void *)mp_esp_dl::detector::face_detector_make_new,
    print, (const void *)mp_esp_dl::detector::print,
    locals_dict, &mp_esp_dl::detector::face_detector_locals_dict
);