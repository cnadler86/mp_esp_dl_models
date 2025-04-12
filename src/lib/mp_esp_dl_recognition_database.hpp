#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/idf_additions.h"
#include "dl_recognition_types.hpp"
#include "dl_tensor_base.hpp"
#include "esp_check.h"
#include "esp_system.h"
#include <algorithm>
#include <list>
#include <vector>

extern "C" {
    #include "py/runtime.h" // Für mp_load_method und mp_call_method_n_kw
    #include "py/obj.h" // Für mp_printf
    #include "mpfile.h"
}

namespace mp_esp_dl {
namespace recognition {

class DataBase {
public:
    DataBase(const char *db_path, int feat_len);
    virtual ~DataBase();
    esp_err_t clear_all_feats();
    esp_err_t enroll_feat(dl::TensorBase *feat, const char *name = "");
    esp_err_t delete_feat(uint16_t id);
    esp_err_t delete_last_feat();
    std::vector<result_t> query_feat(dl::TensorBase *feat, float thr, int top_k);
    const char* get_name(uint16_t id);
    void print();
    int get_num_feats() { return m_meta.num_feats_valid; }

private:
    char *m_db_path;
    std::list<database_feat> m_feats;
    database_meta m_meta;

    esp_err_t create_empty_database_in_storage(int feat_len);
    esp_err_t load_database_from_storage(int feat_len);
    void clear_all_feats_in_memory();
    float cal_similarity(float *feat1, float *feat2);
};

} // namespace recognition
} // namespace mp_esp_dl
