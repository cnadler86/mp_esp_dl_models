#pragma once
#include <cstdint>

namespace mp_esp_dl {
namespace recognition {

struct database_meta {
    uint16_t num_feats_total;
    uint16_t num_feats_valid;
    uint16_t feat_len;
};

struct database_feat {
    uint16_t id;
    float *feat;

    database_feat() : id(0), feat(nullptr) {}
    database_feat(uint16_t id, float *feat) : id(id), feat(feat) {}
};

struct result_t {
    uint16_t id;
    float similarity;

    result_t() : id(0), similarity(0.0f) {}
    result_t(uint16_t id, float similarity) : id(id), similarity(similarity) {}
};

} // namespace recognition
} // namespace dl