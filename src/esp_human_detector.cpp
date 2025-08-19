#include "mp_esp_dl.hpp"
#include "freertos/idf_additions.h"
#include "pedestrian_detect.hpp"

#if MP_DL_PEDESTRIAN_DETECTOR_ENABLED

namespace mp_esp_dl::HumanDetector {

// Object
struct MP_HumanDetector : public MP_DetectorBase<PedestrianDetect> {
};

// Constructor
static mp_obj_t human_detector_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    enum { ARG_img_width, ARG_img_height, ARG_pixel_format };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width, MP_ARG_INT, {.u_int = 320} },
        { MP_QSTR_height, MP_ARG_INT, {.u_int = 240} },
        { MP_QSTR_pixel_format, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = dl::image::DL_IMAGE_PIX_TYPE_RGB888} },
    };

    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    MP_HumanDetector *self = mp_esp_dl::make_new<MP_HumanDetector, PedestrianDetect>(
        &mp_human_detector_type, 
        parsed_args[ARG_img_width].u_int, 
        parsed_args[ARG_img_height].u_int,
        static_cast<dl::image::pix_type_t>(parsed_args[ARG_pixel_format].u_int));

    return MP_OBJ_FROM_PTR(self);
}

// Destructor
static mp_obj_t human_detector_del(mp_obj_t self_in) {
    MP_HumanDetector *self = static_cast<MP_HumanDetector *>(MP_OBJ_TO_PTR(self_in));
    self->model = nullptr;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1_CXX(human_detector_del_obj, human_detector_del);

// Get and set methods
static void human_detector_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    mp_esp_dl::espdl_obj_property<MP_HumanDetector>(self_in, attr, dest);
}

// Detect method
static mp_obj_t human_detector_detect(mp_obj_t self_in, mp_obj_t framebuffer_obj) {
    MP_HumanDetector *self = mp_esp_dl::get_and_validate_framebuffer<MP_HumanDetector>(self_in, framebuffer_obj);

    auto &detect_results = self->model->run(self->img);

    if (detect_results.size() == 0) {
        return mp_const_none;
    }

    mp_obj_t list = mp_obj_new_list(0, NULL);
    for (const auto &res : detect_results) {
        mp_obj_t dict = mp_obj_new_dict(2);
        mp_obj_dict_store(dict, mp_obj_new_str_from_cstr("score"), mp_obj_new_float(res.score));

        mp_obj_t tuple[4];
        for (int i = 0; i < 4; ++i) {
            tuple[i] = mp_obj_new_int(res.box[i]);
        }
        mp_obj_dict_store(dict, mp_obj_new_str_from_cstr("box"), mp_obj_new_tuple(4, tuple));
        
        mp_obj_list_append(list, dict);
    }
    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_2_CXX(human_detector_detect_obj, human_detector_detect);

// Local dict
static const mp_rom_map_elem_t human_detector_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_run), MP_ROM_PTR(&human_detector_detect_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&human_detector_del_obj) },
};
static MP_DEFINE_CONST_DICT(human_detector_locals_dict, human_detector_locals_dict_table);

// Print
static void print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_printf(print, "Human detector object");
}

} //namespace

// Type
MP_DEFINE_CONST_OBJ_TYPE(
    mp_human_detector_type,
    MP_QSTR_HumanDetector,
    MP_TYPE_FLAG_NONE,
    make_new, (const void *)mp_esp_dl::HumanDetector::human_detector_make_new,
    print, (const void *)mp_esp_dl::HumanDetector::print,
    attr, (const void *)mp_esp_dl::HumanDetector::human_detector_attr,
    locals_dict, &mp_esp_dl::HumanDetector::human_detector_locals_dict
);

#endif // MP_DL_PEDESTRIAN_DETECTOR_ENABLED