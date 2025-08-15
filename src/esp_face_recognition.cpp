#include "mp_esp_dl.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "human_face_detect.hpp"
#include "lib/mp_esp_dl_human_face_recognition.hpp"

#if MP_DL_FACE_RECOGNITION_ENABLED

namespace mp_esp_dl::recognition {

// Object
struct MP_FaceRecognizer : public MP_DetectorBase<HumanFaceDetect> {
    std::shared_ptr<HumanFaceFeat> FaceFeat = nullptr;
    std::shared_ptr<HumanFaceRecognizer> FaceRecognizer = nullptr;
    bool return_features;
    char db_path[64];
};

// Constructor
static mp_obj_t face_recognizer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    enum { ARG_img_width, ARG_img_height, ARG_pixel_format, ARG_features, ARG_db_path, ARG_model };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_width, MP_ARG_INT, {.u_int = 320} },
        { MP_QSTR_height, MP_ARG_INT, {.u_int = 240} },
        { MP_QSTR_pixel_format, MP_ARG_INT, {.u_int = dl::image::DL_IMAGE_PIX_TYPE_RGB888} },
        { MP_QSTR_features, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = true} },
        { MP_QSTR_db_path, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    #if CONFIG_HUMAN_FACE_FEAT_MFN_S8_V1 && CONFIG_HUMAN_FACE_FEAT_MBF_S8_V1
        { MP_QSTR_model, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    #endif
    };

    mp_arg_val_t parsed_args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, args, MP_ARRAY_SIZE(allowed_args), allowed_args, parsed_args);

    MP_FaceRecognizer *self = mp_esp_dl::make_new<MP_FaceRecognizer, HumanFaceDetect>(
        &mp_face_recognizer_type, 
        parsed_args[ARG_img_width].u_int, 
        parsed_args[ARG_img_height].u_int,
        static_cast<dl::image::pix_type_t>(parsed_args[ARG_pixel_format].u_int));

    strncpy(self->db_path, "/face.db", sizeof(self->db_path));
    if (parsed_args[ARG_db_path].u_obj != mp_const_none) {
        snprintf(self->db_path, sizeof(self->db_path), "/%s", mp_obj_str_get_str(parsed_args[ARG_db_path].u_obj));
    }

#if CONFIG_HUMAN_FACE_FEAT_MFN_S8_V1 && CONFIG_HUMAN_FACE_FEAT_MBF_S8_V1
    if (parsed_args[ARG_model].u_obj == mp_const_none) {
#endif
        self->FaceFeat = std::make_shared<HumanFaceFeat>();
#if CONFIG_HUMAN_FACE_FEAT_MFN_S8_V1 && CONFIG_HUMAN_FACE_FEAT_MBF_S8_V1
    } else {
        const char *model = mp_obj_str_get_str(parsed_args[ARG_model].u_obj);
        if (strcmp(model, "MBF") == 0) {
            self->FaceFeat = std::make_shared<HumanFaceFeat>(HumanFaceFeat::MBF_S8_V1);
        } else if (strcmp(model, "MFN") == 0) {
            self->FaceFeat = std::make_shared<HumanFaceFeat>(HumanFaceFeat::MFN_S8_V1);
        } else {
            mp_printf(&mp_plat_print, "Model %s invalid. Using default feature model\n", model);
            self->FaceFeat = std::make_shared<HumanFaceFeat>();
        }
    }
#endif
    self->FaceRecognizer = std::make_shared<HumanFaceRecognizer>(self->FaceFeat.get(), self->db_path);

    if ((!self->FaceFeat) || (!self->FaceRecognizer)) {
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed to create model instances"));
    }

    self->return_features = parsed_args[ARG_features].u_bool;

    return MP_OBJ_FROM_PTR(self);
}

// Destructor
static mp_obj_t face_recognizer_del(mp_obj_t self_in) {
    MP_FaceRecognizer *self = static_cast<MP_FaceRecognizer *>(MP_OBJ_TO_PTR(self_in));
    self->model = nullptr;
    self->FaceFeat = nullptr;
    self->FaceRecognizer = nullptr;
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(face_recognizer_del_obj, face_recognizer_del);

// Get and set methods
static void face_recognizer_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
    mp_esp_dl::espdl_obj_property<MP_FaceRecognizer>(self_in, attr, dest);
}

// Enroll method
static mp_obj_t face_recognizer_enroll(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_self, ARG_framebuffer, ARG_validate, ARG_name };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },  // self
        { MP_QSTR_, MP_ARG_REQUIRED | MP_ARG_OBJ },  // framebuffer
        { MP_QSTR_validate, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_name, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    MP_FaceRecognizer *self = mp_esp_dl::get_and_validate_framebuffer<MP_FaceRecognizer>(args[ARG_self].u_obj, args[ARG_framebuffer].u_obj);
    bool validate = args[ARG_validate].u_bool;

    auto &detect_results = self->model->run(self->img);

    if (detect_results.size() == 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("No face detected."));
    }
    if (detect_results.size() > 1) {
        mp_raise_ValueError(MP_ERROR_TEXT("Only one face can be enrolled at a time."));
    }
    
    // Only validate if explicitly requested
    if (validate) {
        auto recon_results = self->FaceRecognizer->recognize(self->img, detect_results);
        if (!recon_results.empty() && recon_results[0].similarity > 0.9) {
            mp_warning("espdl", "Face already enrolled. id: %d, similarity: %f", recon_results[0].id, recon_results[0].similarity);
            return mp_const_none;
        }
    }

    const char* name = "";
    if (args[ARG_name].u_obj != mp_const_none) {
        name = mp_obj_str_get_str(args[ARG_name].u_obj);
    }

    uint16_t new_id;
    if (self->FaceRecognizer->enroll(self->img, detect_results, name, &new_id) != ESP_OK) {
        mp_raise_ValueError(MP_ERROR_TEXT("Failed to enroll face."));
    }

    return mp_obj_new_int(new_id);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(face_recognizer_enroll_obj, 2, face_recognizer_enroll);

// Delete feature method
static mp_obj_t face_recognizer_delete_feature(mp_obj_t self_in, mp_obj_t id) {
    MP_FaceRecognizer *self = static_cast<MP_FaceRecognizer *>(MP_OBJ_TO_PTR(self_in));
    int id_num = mp_obj_get_int(id);
    if (self->FaceRecognizer->delete_feat(id_num) != ESP_OK) {
        mp_raise_ValueError(MP_ERROR_TEXT("Failed to delete feature."));
    }
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(face_recognizer_delete_feature_obj, face_recognizer_delete_feature);

// Recognize method
static mp_obj_t face_recognizer_recognize(mp_obj_t self_in, mp_obj_t framebuffer_obj) {
    MP_FaceRecognizer *self = mp_esp_dl::get_and_validate_framebuffer<MP_FaceRecognizer>(self_in, framebuffer_obj);

    auto &detect_results = self->model->run(self->img);

    if (detect_results.size() == 0) {
        return mp_const_none;
    }

    mp_obj_t list = mp_obj_new_list(0, NULL);
    for (const auto &res : detect_results) {
        mp_obj_t dict = mp_obj_new_dict(4);
        mp_obj_dict_store(dict, mp_obj_new_str_from_cstr("score"), mp_obj_new_float(res.score));

        mp_obj_t tuple[4];
        for (int i = 0; i < 4; ++i) {
            tuple[i] = mp_obj_new_int(res.box[i]);
        }
        mp_obj_dict_store(dict, mp_obj_new_str_from_cstr("box"), mp_obj_new_tuple(4, tuple));
    
        if (self->return_features) {
            mp_obj_t features[10];
            for (int i = 0; i < 10; ++i) {
                features[i] = mp_obj_new_int(res.keypoint[i]);
            }
            mp_obj_dict_store(dict, mp_obj_new_str_from_cstr("features"), mp_obj_new_tuple(10, features));
        }
        else {
            mp_obj_dict_store(dict, mp_obj_new_str_from_cstr("features"), mp_const_none);
        }
        
        std::list<dl::detect::result_t> single_result_list = { res };
        auto recon_results = self->FaceRecognizer->recognize(self->img, single_result_list);
        if(recon_results.size() == 0) {
            mp_obj_dict_store(dict, mp_obj_new_str_from_cstr("person"), mp_const_none);
        } else {
            mp_obj_t person_dict = mp_obj_new_dict(3);
            mp_obj_dict_store(person_dict, mp_obj_new_str_from_cstr("id"), mp_obj_new_int(recon_results[0].id));
            mp_obj_dict_store(person_dict, mp_obj_new_str_from_cstr("similarity"), mp_obj_new_float(recon_results[0].similarity));
            
            // Füge den Namen hinzu, wenn er nicht leer ist
            if (recon_results[0].name[0] != '\0') {
                mp_obj_dict_store(person_dict, mp_obj_new_str_from_cstr("name"), 
                                mp_obj_new_str(recon_results[0].name, strlen(recon_results[0].name)));
            } else {
                mp_obj_dict_store(person_dict, mp_obj_new_str_from_cstr("name"), mp_const_none);
            }
            
            mp_obj_dict_store(dict, mp_obj_new_str_from_cstr("person"), person_dict);
        }
        mp_obj_list_append(list, dict);
    }
    return list;
}
static MP_DEFINE_CONST_FUN_OBJ_2(face_recognizer_recognize_obj, face_recognizer_recognize);

// Print Database
static mp_obj_t face_recognizer_print_database(mp_obj_t self_in) {
    MP_FaceRecognizer *self = static_cast<MP_FaceRecognizer *>(MP_OBJ_TO_PTR(self_in));
    self->FaceRecognizer->print();
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(face_recognizer_print_database_obj, face_recognizer_print_database);

// Local dict
static const mp_rom_map_elem_t face_recognizer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_run), MP_ROM_PTR(&face_recognizer_recognize_obj) },
    { MP_ROM_QSTR(MP_QSTR_enroll), MP_ROM_PTR(&face_recognizer_enroll_obj) },
    { MP_ROM_QSTR(MP_QSTR_delete_face), MP_ROM_PTR(&face_recognizer_delete_feature_obj) },
    { MP_ROM_QSTR(MP_QSTR_print_database), MP_ROM_PTR(&face_recognizer_print_database_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&face_recognizer_del_obj) },
};
static MP_DEFINE_CONST_DICT(face_recognizer_locals_dict, face_recognizer_locals_dict_table);

// Print
static void print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    MP_FaceRecognizer *self = static_cast<MP_FaceRecognizer *>(MP_OBJ_TO_PTR(self_in));
    mp_printf(print, "Face recognition object with total of %d features", self->FaceRecognizer->get_num_feats());
}

} //namespace

// Type
MP_DEFINE_CONST_OBJ_TYPE(
    mp_face_recognizer_type,
    MP_QSTR_FaceRecognizer,
    MP_TYPE_FLAG_NONE,
    make_new, (const void *)mp_esp_dl::recognition::face_recognizer_make_new,
    print, (const void *)mp_esp_dl::recognition::print,
    attr, (const void *)mp_esp_dl::recognition::face_recognizer_attr,
    locals_dict, &mp_esp_dl::recognition::face_recognizer_locals_dict
);

#endif // MP_DL_FACE_RECOGNITION_ENABLED