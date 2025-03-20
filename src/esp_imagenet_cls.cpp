#include "esp_mp_dl.h"
#include "freertos/idf_additions.h"
#include "imagenet_cls.hpp"
#include <memory>

#if MP_DL_IMAGENET_CLS_ENABLED

namespace mp_dl::imagenet {

// Object
struct MP_ImageNetCls {
    mp_obj_base_t base;
    std::shared_ptr<ImageNetCls> model = nullptr;
    dl::image::img_t img;
};

// Constructor
static mp_obj_t image_net_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    enum { ARG_img_width, ARG_img_height };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width, MP_ARG_INT, {.u_int = 320} },
        { MP_QSTR_height, MP_ARG_INT, {.u_int = 240} },
    };

    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    MP_ImageNetCls *self = mp_obj_malloc_with_finaliser(MP_ImageNetCls, &mp_image_net_type);
    self->model = std::make_shared<ImageNetCls>();
    
    if (!self->model) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to create model instance");
    }

    initialize_img(self->img, parsed_args[ARG_img_width].u_int, parsed_args[ARG_img_height].u_int);

    return MP_OBJ_FROM_PTR(self);
}

// Destructor
static mp_obj_t image_net_del(mp_obj_t self_in) {
    MP_ImageNetCls *self = static_cast<MP_ImageNetCls *>(MP_OBJ_TO_PTR(self_in));
    self->model = nullptr;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1_CXX(image_net_del_obj, image_net_del);

// Detect method
static mp_obj_t image_net_detect(mp_obj_t self_in, mp_obj_t framebuffer_obj) {
    MP_ImageNetCls *self = static_cast<MP_ImageNetCls *>(MP_OBJ_TO_PTR(self_in));

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
        mp_obj_list_append(list, mp_obj_new_str_from_cstr(res.cat_name));
        mp_obj_list_append(list, mp_obj_new_float(res.score));
    }
    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_2_CXX(image_net_detect_obj, image_net_detect);

// Local dict
static const mp_rom_map_elem_t image_net_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_detect), MP_ROM_PTR(&image_net_detect_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&image_net_del_obj) },
};
static MP_DEFINE_CONST_DICT(image_net_locals_dict, image_net_locals_dict_table);

// Print
static void print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    mp_printf(print, "Face detector object");
}

} //namespace

// Type
MP_DEFINE_CONST_OBJ_TYPE(
    mp_image_net_type,
    MP_QSTR_ImageNet,
    MP_TYPE_FLAG_NONE,
    make_new, (const void *)mp_dl::imagenet::image_net_make_new,
    print, (const void *)mp_dl::imagenet::print,
    locals_dict, &mp_dl::imagenet::image_net_locals_dict
);

#endif // MP_DL_IMAGENET_CLS_ENABLED