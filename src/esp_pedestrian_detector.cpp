#include "mp_esp_dl.hpp"
#include "freertos/idf_additions.h"
#include "pedestrian_detect.hpp"

#if MP_DL_PEDESTRISN_DETECTOR_ENABLED

namespace mp_esp_dl::PedestrianDetector {

// Object
struct MP_PedestrianDetector {
    mp_obj_base_t base;
    std::shared_ptr<PedestrianDetect> model = nullptr;
    dl::image::img_t img;
    bool return_features;
};

// Constructor
static mp_obj_t pedestrian_detector_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    enum { ARG_img_width, ARG_img_height, ARG_return_features };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width, MP_ARG_INT, {.u_int = 320} },
        { MP_QSTR_height, MP_ARG_INT, {.u_int = 240} },
    };

    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    MP_PedestrianDetector *self = mp_obj_malloc_with_finaliser(MP_PedestrianDetector, &mp_pedestrian_detector_type);
    self->model = std::make_shared<PedestrianDetect>();
    
    if (!self->model) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to create model instance");
    }

    mp_esp_dl::initialize_img(self->img, parsed_args[ARG_img_width].u_int, parsed_args[ARG_img_height].u_int);

    return MP_OBJ_FROM_PTR(self);
}

// Destructor
static mp_obj_t pedestrian_detector_del(mp_obj_t self_in) {
    MP_PedestrianDetector *self = static_cast<MP_PedestrianDetector *>(MP_OBJ_TO_PTR(self_in));
    self->model = nullptr;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1_CXX(pedestrian_detector_del_obj, pedestrian_detector_del);

// Get and set methods
static void pedestrian_detector_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    mp_esp_dl::espdl_obj_property<MP_PedestrianDetector>(self_in, attr, dest);
}

// Detect method
static mp_obj_t pedestrian_detector_detect(mp_obj_t self_in, mp_obj_t framebuffer_obj) {
    MP_PedestrianDetector *self = mp_esp_dl::get_and_validate_framebuffer<MP_PedestrianDetector>(self_in, framebuffer_obj);

    auto &detect_results = self->model->run(self->img);

    if (detect_results.size() == 0) {
        return mp_const_none;
    }

    mp_obj_t list = mp_obj_new_list(0, NULL);
    for (const auto &res : detect_results) {
        mp_obj_list_append(list, mp_obj_new_float(res.score));
        mp_obj_t tuple[4];
        for (int i = 0; i < 4; ++i) {
            tuple[i] = mp_obj_new_int(res.box[i]);
        }
        mp_obj_list_append(list, mp_obj_new_tuple(4, tuple));
    }
    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_2_CXX(pedestrian_detector_detect_obj, pedestrian_detector_detect);

// Local dict
static const mp_rom_map_elem_t pedestrian_detector_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_run), MP_ROM_PTR(&pedestrian_detector_detect_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&pedestrian_detector_del_obj) },
};
static MP_DEFINE_CONST_DICT(pedestrian_detector_locals_dict, pedestrian_detector_locals_dict_table);

// Print
static void print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_printf(print, "Pedestrian detector object");
}

} //namespace

// Type
MP_DEFINE_CONST_OBJ_TYPE(
    mp_pedestrian_detector_type,
    MP_QSTR_PedestrianDetector,
    MP_TYPE_FLAG_NONE,
    make_new, (const void *)mp_esp_dl::PedestrianDetector::pedestrian_detector_make_new,
    print, (const void *)mp_esp_dl::PedestrianDetector::print,
    attr, (const void *)mp_esp_dl::PedestrianDetector::pedestrian_detector_attr,
    locals_dict, &mp_esp_dl::PedestrianDetector::pedestrian_detector_locals_dict
);

#endif // MP_DL_PEDESTRISN_DETECTOR_ENABLED