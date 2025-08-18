#include "py/runtime.h"
#include "mp_esp_dl.hpp"

static const mp_rom_map_elem_t module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_espdl) },
    #if MP_DL_FACE_DETECTOR_ENABLED
    { MP_ROM_QSTR(MP_QSTR_FaceDetector), MP_ROM_PTR(&mp_face_detector_type) },
    #endif
    #if MP_DL_FACE_RECOGNITION_ENABLED
    { MP_ROM_QSTR(MP_QSTR_FaceRecognizer), MP_ROM_PTR(&mp_face_recognizer_type) },
    #endif
    #if MP_DL_PEDESTRIAN_DETECTOR_ENABLED
    { MP_ROM_QSTR(MP_QSTR_HumanDetector), MP_ROM_PTR(&mp_human_detector_type) },
    #endif
    #if MP_DL_IMAGENET_CLS_ENABLED
    { MP_ROM_QSTR(MP_QSTR_ImageNet), MP_ROM_PTR(&mp_image_net_type) },
    #endif
    #if MP_DL_COCO_DETECTOR_ENABLED
    { MP_ROM_QSTR(MP_QSTR_CocoDetector), MP_ROM_PTR(&mp_coco_detector_type) },
    #endif
    #if MP_DL_CAT_DETECTOR_ENABLED
    { MP_ROM_QSTR(MP_QSTR_CatDetector), MP_ROM_PTR(&mp_cat_detector_type) },
    #endif
};
static MP_DEFINE_CONST_DICT(module_globals, module_globals_table);

const mp_obj_module_t mp_esp_dl_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&module_globals,
};
MP_REGISTER_MODULE(MP_QSTR_espdl, mp_esp_dl_module);