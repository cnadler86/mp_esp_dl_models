#include "mp_esp_dl.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "human_face_detect.hpp"
// #include "human_face_recognition.hpp"
#include "lib/mp_esp_dl_human_face_recognition.hpp"
#if MP_DL_FACE_RECOGNITION_ENABLED

namespace mp_esp_dl::recognition {

// Object
struct MP_FaceRecognizer {
    mp_obj_base_t base;
    std::shared_ptr<HumanFaceDetect> FaceDetector;
    std::shared_ptr<HumanFaceFeat> FaceFeat;
    std::shared_ptr<HumanFaceRecognizer> FaceRecognizer;
    dl::image::img_t img;
    bool return_features;   
    char db_path[64];
};

// Constructor
static mp_obj_t face_recognizer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    enum { ARG_img_width, ARG_img_height, ARG_return_features, ARG_db_path };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 320} },
        { MP_QSTR_height, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 240} },
        { MP_QSTR_features, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_db_path, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    MP_FaceRecognizer *self = mp_obj_malloc_with_finaliser(MP_FaceRecognizer, &mp_face_recognizer_type);

    strncpy(self->db_path, "/face.db", sizeof(self->db_path));
    if (parsed_args[ARG_db_path].u_obj != mp_const_none) {
        snprintf(self->db_path, sizeof(self->db_path), "/%s", mp_obj_str_get_str(parsed_args[ARG_db_path].u_obj));
    }
    
    // ESP_ERROR_CHECK(fatfs_flash_mount());

    self->FaceDetector = std::make_shared<HumanFaceDetect>();
    self->FaceFeat = std::make_shared<HumanFaceFeat>();
    self->FaceRecognizer = std::make_shared<HumanFaceRecognizer>(self->FaceFeat.get(), self->db_path);

    if ((!self->FaceDetector) || (!self->FaceFeat) || (!self->FaceRecognizer)) {
        mp_raise_msg(&mp_type_RuntimeError, "Failed to create model instances");
    }

    mp_esp_dl::initialize_img(self->img, parsed_args[ARG_img_width].u_int, parsed_args[ARG_img_height].u_int);

    self->return_features = parsed_args[ARG_return_features].u_bool;

    return MP_OBJ_FROM_PTR(self);
}

// Destructor
static mp_obj_t face_recognizer_del(mp_obj_t self_in) {
    MP_FaceRecognizer *self = static_cast<MP_FaceRecognizer *>(MP_OBJ_TO_PTR(self_in));
    self->FaceDetector = nullptr;
    self->FaceFeat = nullptr;
    self->FaceRecognizer = nullptr;
    // ESP_ERROR_CHECK(fatfs_flash_unmount());
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1_CXX(face_recognizer_del_obj, face_recognizer_del);

// Set width and height
static mp_obj_t face_recognizer_set_pixelformat(mp_obj_t self_in, mp_obj_t width_obj, mp_obj_t height_obj) {
    set_width_and_height<MP_FaceRecognizer>(self_in, mp_obj_get_int(width_obj), mp_obj_get_int(height_obj));
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3_CXX(face_recognizer_set_pixelformat_obj, face_recognizer_set_pixelformat);

// Enroll method
static mp_obj_t face_recognizer_enroll(mp_obj_t self_in, mp_obj_t framebuffer_obj) {
    MP_FaceRecognizer *self = mp_esp_dl::get_and_validate_framebuffer<MP_FaceRecognizer>(self_in, framebuffer_obj);

    auto &detect_results = self->FaceDetector->run(self->img);

    if (detect_results.size() == 0) {
        mp_raise_ValueError("No face detected");
    }
    if (detect_results.size() > 1) {
        mp_raise_ValueError("Only one face can be enrolled at a time");
    }

    self->FaceRecognizer->enroll(self->img, detect_results);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2_CXX(face_recognizer_enroll_obj, face_recognizer_enroll);

// Detect method
static mp_obj_t face_recognizer_recognize(mp_obj_t self_in, mp_obj_t framebuffer_obj) {
    MP_FaceRecognizer *self = mp_esp_dl::get_and_validate_framebuffer<MP_FaceRecognizer>(self_in, framebuffer_obj);

    auto &detect_results = self->FaceDetector->run(self->img);

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
        
        std::list<dl::detect::result_t> single_result_list = { res };
        auto recon_results = self->FaceRecognizer->recognize(self->img, single_result_list);

        if(recon_results.size() == 0) {
            mp_obj_list_append(list, mp_const_none);
        } else {
            for (const auto &recon : recon_results) {
                mp_obj_t tuple[2];
                tuple[0] = mp_obj_new_int(recon.id);
                tuple[1] = mp_obj_new_float(recon.similarity);
                mp_obj_list_append(list, mp_obj_new_tuple(2, tuple));
            }
        }

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
static MP_DEFINE_CONST_FUN_OBJ_2_CXX(face_recognizer_recognize_obj, face_recognizer_recognize);

// Print Database
static mp_obj_t face_recognizer_print_database(mp_obj_t self_in) {
    MP_FaceRecognizer *self = static_cast<MP_FaceRecognizer *>(MP_OBJ_TO_PTR(self_in));
    self->FaceRecognizer->print();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1_CXX(face_recognizer_print_database_obj, face_recognizer_print_database);

// Local dict
static const mp_rom_map_elem_t face_recognizer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_recognize), MP_ROM_PTR(&face_recognizer_recognize_obj) },
    { MP_ROM_QSTR(MP_QSTR_enroll), MP_ROM_PTR(&face_recognizer_enroll_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixelformat), MP_ROM_PTR(&face_recognizer_set_pixelformat_obj) },
    { MP_ROM_QSTR(MP_QSTR_print_database), MP_ROM_PTR(&face_recognizer_print_database_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&face_recognizer_del_obj) },
};
static MP_DEFINE_CONST_DICT(face_recognizer_locals_dict, face_recognizer_locals_dict_table);

// Print
static void print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    mp_printf(print, "Face recognition object");
}

} //namespace

// Type
MP_DEFINE_CONST_OBJ_TYPE(
    mp_face_recognizer_type,
    MP_QSTR_FaceRecognizer,
    MP_TYPE_FLAG_NONE,
    make_new, (const void *)mp_esp_dl::recognition::face_recognizer_make_new,
    print, (const void *)mp_esp_dl::recognition::print,
    locals_dict, &mp_esp_dl::recognition::face_recognizer_locals_dict
);

#endif // MP_DL_FACE_RECOGNITION_ENABLED