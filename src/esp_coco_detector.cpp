#include "mp_esp_dl.hpp"
#include "freertos/idf_additions.h"
#include "coco_detect.hpp"

#if MP_DL_COCO_DETECTOR_ENABLED

namespace mp_esp_dl::CocoDetector {

// Object
struct MP_CocoDetector : public MP_DetectorBase<COCODetect> {
};

// Constructor
static mp_obj_t coco_detector_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    enum { ARG_img_width, ARG_img_height, ARG_pixel_format, ARG_model };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width, MP_ARG_INT, {.u_int = 320} },
        { MP_QSTR_height, MP_ARG_INT, {.u_int = 240} },
        { MP_QSTR_pixel_format, MP_ARG_INT, {.u_int = dl::image::DL_IMAGE_PIX_TYPE_RGB888} },
        { MP_QSTR_model, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = CONFIG_DEFAULT_COCO_DETECT_MODEL} },
    };

    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    MP_CocoDetector *self = mp_esp_dl::make_new<MP_CocoDetector, COCODetect>(
        &mp_coco_detector_type, 
        parsed_args[ARG_img_width].u_int, 
        parsed_args[ARG_img_height].u_int,
        static_cast<dl::image::pix_type_t>(parsed_args[ARG_pixel_format].u_int),
        static_cast<COCODetect::model_type_t>(parsed_args[ARG_model].u_int));

    return MP_OBJ_FROM_PTR(self);
}

// Destructor
static mp_obj_t coco_detector_del(mp_obj_t self_in) {
    MP_CocoDetector *self = static_cast<MP_CocoDetector *>(MP_OBJ_TO_PTR(self_in));
    self->model = nullptr;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(coco_detector_del_obj, coco_detector_del);

// Get and set methods
static void coco_detector_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest){
    mp_esp_dl::espdl_obj_property<MP_CocoDetector>(self_in, attr, dest);
}

// classify method
static mp_obj_t coco_detector_detect(mp_obj_t self_in, mp_obj_t framebuffer_obj) {
    MP_CocoDetector *self = mp_esp_dl::get_and_validate_framebuffer<MP_CocoDetector>(self_in, framebuffer_obj);

    auto &detect_results = self->model->run(self->img);

    if (detect_results.size() == 0) {
        return mp_const_none;
    }

    mp_obj_t list = mp_obj_new_list(0, NULL);
    for (const auto &res : detect_results) {
        mp_obj_t dict = mp_obj_new_dict(3);
        mp_obj_dict_store(dict, mp_obj_new_str_from_cstr("category"), mp_obj_new_int(res.category));
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
static MP_DEFINE_CONST_FUN_OBJ_2(coco_detector_detect_obj, coco_detector_detect);

// Local dict
static const mp_rom_map_elem_t coco_detector_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_run), MP_ROM_PTR(&coco_detector_detect_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&coco_detector_del_obj) },
};
static MP_DEFINE_CONST_DICT(coco_detector_locals_dict, coco_detector_locals_dict_table);

// Print
static void print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_printf(print, "Coco detector object");
}

} //namespace

// Type
MP_DEFINE_CONST_OBJ_TYPE(
    mp_coco_detector_type,
    MP_QSTR_CocoDetector,
    MP_TYPE_FLAG_NONE,
    make_new, (const void *)mp_esp_dl::CocoDetector::coco_detector_make_new,
    print, (const void *)mp_esp_dl::CocoDetector::print,
    attr, (const void *)mp_esp_dl::CocoDetector::coco_detector_attr,
    locals_dict, &mp_esp_dl::CocoDetector::coco_detector_locals_dict
);

#endif // MP_DL_COCO_DETECTOR_ENABLED