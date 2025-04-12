#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/idf_additions.h"
#include "mp_esp_dl_recognition_database.hpp"
#include "dl_detect_define.hpp"
#include "dl_feat_base.hpp"
#include "dl_tensor_base.hpp"
namespace human_face_recognition {
class MFN : public dl::feat::FeatImpl {
public:
    MFN(const char *model_name);
};

using MBF = MFN;
} // namespace human_face_recognition

class HumanFaceFeat : public dl::feat::FeatWrapper {
public:
    typedef enum {
        MFN_S8_V1,
        MBF_S8_V1,
    } model_type_t;
    HumanFaceFeat(model_type_t model_type = static_cast<model_type_t>(CONFIG_HUMAN_FACE_FEAT_MODEL_TYPE));
};

class HumanFaceRecognizer : public mp_esp_dl::recognition::DataBase {
private:
    HumanFaceFeat *m_feat_extract;
    float m_thr;
    int m_top_k;

public:
    HumanFaceRecognizer(HumanFaceFeat *feat_model, char *db_path, float thr = 0.5, int top_k = 1) :
        mp_esp_dl::recognition::DataBase(db_path, feat_model->m_feat_len),
        m_feat_extract(feat_model),
        m_thr(thr),
        m_top_k(top_k)
    {
    }

    std::vector<mp_esp_dl::recognition::result_t> recognize(const dl::image::img_t &img,
                                                     std::list<dl::detect::result_t> &detect_res);
    esp_err_t enroll(const dl::image::img_t &img, std::list<dl::detect::result_t> &detect_res, const char *name = "");
};
