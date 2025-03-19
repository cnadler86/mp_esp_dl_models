#include "py/runtime.h"
#include "esp_mp_dl.h"

static const mp_rom_map_elem_t module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mp_dl) },
    { MP_ROM_QSTR(MP_QSTR_FaceDetector), MP_ROM_PTR(&mp_face_detector_type) },
    { MP_ROM_QSTR(MP_QSTR_ImageNet), MP_ROM_PTR(&mp_image_net_type) },
};
static MP_DEFINE_CONST_DICT(module_globals, module_globals_table);

const mp_obj_module_t face_detector_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&module_globals,
};
MP_REGISTER_MODULE(MP_QSTR_mp_dl, face_detector_module);