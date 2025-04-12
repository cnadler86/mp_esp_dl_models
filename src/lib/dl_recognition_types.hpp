#pragma once
#include <cstdint>
#include <cstring>

namespace mp_esp_dl {
namespace recognition {

#define MAX_NAME_LENGTH 32

struct database_meta {
    uint16_t num_feats_total;
    uint16_t num_feats_valid;
    uint16_t feat_len;
};

struct database_feat {
    uint16_t id;
    float *feat;
    char name[MAX_NAME_LENGTH];

    database_feat() : id(0), feat(nullptr) {
        name[0] = '\0';
    }
    
    database_feat(uint16_t _id, float *_feat, const char *_name = "") : id(_id), feat(_feat) {
        strncpy(name, _name, MAX_NAME_LENGTH - 1);
        name[MAX_NAME_LENGTH - 1] = '\0';
    }
};

struct result_t {
    uint16_t id;
    float similarity;
    char name[MAX_NAME_LENGTH];

    result_t() : id(0), similarity(0.0f) {
        name[0] = '\0';
    }
    
    result_t(uint16_t _id, float _sim, const char *_name = "") : id(_id), similarity(_sim) {
        strncpy(name, _name, MAX_NAME_LENGTH - 1);
        name[MAX_NAME_LENGTH - 1] = '\0';
    }
};

} // namespace recognition
} // namespace mp_esp_dl