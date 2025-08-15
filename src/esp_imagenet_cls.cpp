#include "mp_esp_dl.hpp"
#include "freertos/idf_additions.h"
#include "imagenet_cls.hpp"

#if MP_DL_IMAGENET_CLS_ENABLED

namespace mp_esp_dl::imagenet {

// Object
struct MP_ImageNetCls : public MP_DetectorBase<ImageNetCls> {
};

// Constructor
static mp_obj_t image_net_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    enum { ARG_img_width, ARG_img_height, ARG_pixel_format };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width, MP_ARG_INT, {.u_int = 320} },
        { MP_QSTR_height, MP_ARG_INT, {.u_int = 240} },
        { MP_QSTR_pixel_format, MP_ARG_INT, {.u_int = dl::image::DL_IMAGE_PIX_TYPE_RGB888} },
    };

    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    MP_ImageNetCls *self = mp_esp_dl::make_new<MP_ImageNetCls, ImageNetCls>(
        &mp_image_net_type, 
        parsed_args[ARG_img_width].u_int, 
        parsed_args[ARG_img_height].u_int,
        static_cast<dl::image::pix_type_t>(parsed_args[ARG_pixel_format].u_int));
    return MP_OBJ_FROM_PTR(self);
}

// Destructor
static mp_obj_t image_net_del(mp_obj_t self_in) {
    MP_ImageNetCls *self = static_cast<MP_ImageNetCls *>(MP_OBJ_TO_PTR(self_in));
    self->model = nullptr;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(image_net_del_obj, image_net_del);

// Get and set methods
static void image_net_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    mp_esp_dl::espdl_obj_property<MP_ImageNetCls>(self_in, attr, dest);
}

// classify method
static mp_obj_t image_net_classify(mp_obj_t self_in, mp_obj_t framebuffer_obj) {
    MP_ImageNetCls *self = mp_esp_dl::get_and_validate_framebuffer<MP_ImageNetCls>(self_in, framebuffer_obj);

    auto &classify_results = self->model->run(self->img);

    if (classify_results.size() == 0) {
        return mp_const_none;
    }

    mp_obj_t list = mp_obj_new_list(0, NULL);
    for (const auto &res : classify_results) {
        mp_obj_list_append(list, mp_obj_new_str_from_cstr(res.cat_name));
        mp_obj_list_append(list, mp_obj_new_float(res.score));
    }
    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_2(image_net_classify_obj, image_net_classify);

// Local dict
static const mp_rom_map_elem_t image_net_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_run), MP_ROM_PTR(&image_net_classify_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&image_net_del_obj) },
};
static MP_DEFINE_CONST_DICT(image_net_locals_dict, image_net_locals_dict_table);

// Print
static void print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    mp_printf(print, "Imagenet classifier object");
}

} //namespace

// Type
MP_DEFINE_CONST_OBJ_TYPE(
    mp_image_net_type,
    MP_QSTR_ImageNet,
    MP_TYPE_FLAG_NONE,
    make_new, (const void *)mp_esp_dl::imagenet::image_net_make_new,
    print, (const void *)mp_esp_dl::imagenet::print,
    attr, (const void *)mp_esp_dl::imagenet::image_net_attr,
    locals_dict, &mp_esp_dl::imagenet::image_net_locals_dict
);

#endif // MP_DL_IMAGENET_CLS_ENABLED