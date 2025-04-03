#include "mp_esp_dl_recognition_database.hpp"
#include <unistd.h>

extern "C" {
    #include "py/runtime.h" // Für mp_load_method und mp_call_method_n_kw
    #include "py/obj.h" // Für mp_printf
    #include "mpfile.h"
}

static const char *TAG = "mp_esp_dl::recognition::DataBase";

namespace mp_esp_dl {
namespace recognition {
DataBase::DataBase(const char *db_path, int feat_len)
{
    assert(db_path);
    int length = strlen(db_path) + 1;
    m_db_path = (char *)malloc(sizeof(char) * length);
    memcpy(m_db_path, db_path, length);
    if (mp_isfile(db_path)) {
        load_database_from_storage(feat_len);
    } else {
        create_empty_database_in_storage(feat_len);
    }
}

DataBase::~DataBase()
{
    clear_all_feats_in_memory();
    free(m_db_path);
}

esp_err_t DataBase::create_empty_database_in_storage(int feat_len)
{   
    ESP_LOGI(TAG, "Creating empty database in storage at location %s with feture len %d.", m_db_path, feat_len);
    mp_file_t *f = mp_open(m_db_path, "wb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open db.");
        return ESP_FAIL;
    }
    m_meta.num_feats_total = 0;
    m_meta.num_feats_valid = 0;
    m_meta.feat_len = feat_len;
    mp_int_t nbytes = mp_write(f, &m_meta, sizeof(mp_esp_dl::recognition::database_meta));
    if (nbytes != sizeof(mp_esp_dl::recognition::database_meta)) {
        ESP_LOGE(TAG, "Failed to write database meta.");
        mp_close(f);
        return ESP_FAIL;
    }
    mp_close(f);
    return ESP_OK;
}

void DataBase::clear_all_feats_in_memory()
{
    for (auto it = m_feats.begin(); it != m_feats.end(); it++) {
        heap_caps_free(it->feat);
    }
    m_feats.clear();
    m_meta.num_feats_total = 0;
    m_meta.num_feats_valid = 0;
}

esp_err_t DataBase::load_database_from_storage(int feat_len)
{
    ESP_LOGI(TAG, "Loading database from storage.");
    clear_all_feats_in_memory();

    // Öffne die Datei mit `mp_open`
    mp_file_t *f = mp_open(m_db_path, "rb");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open db.");
        return ESP_FAIL;
    }

    // Lese die Metadaten aus der Datei
    mp_int_t size = mp_readinto(f, &m_meta, sizeof(database_meta));
    if (size != sizeof(database_meta)) {
        ESP_LOGE(TAG, "Failed to read database meta.");
        mp_close(f);
        return ESP_FAIL;
    }

    // Überprüfe die Feature-Länge
    if (feat_len != m_meta.feat_len) {
        ESP_LOGE(TAG, "Feature len in storage does not match feature len in db.");
        mp_close(f);
        return ESP_FAIL;
    }

    uint16_t id;
    for (int i = 0; i < m_meta.num_feats_total; i++) {
        // Lese die Feature-ID
        size = mp_readinto(f, &id, sizeof(uint16_t));
        if (size != sizeof(uint16_t)) {
            ESP_LOGE(TAG, "Failed to read feature id.");
            mp_close(f);
            return ESP_FAIL;
        }

        // Überspringe ungültige IDs
        if (id == 0) {
            if (mp_seek(f, sizeof(float) * m_meta.feat_len, SEEK_CUR) < 0) {
                ESP_LOGE(TAG, "Failed to seek db file.");
                mp_close(f);
                return ESP_FAIL;
            }
            continue;
        }

        // Lese das Feature
        float *feat = (float *)heap_caps_malloc(m_meta.feat_len * sizeof(float), MALLOC_CAP_SPIRAM);
        size = mp_readinto(f, feat, sizeof(float) * m_meta.feat_len);
        if (size != (mp_int_t)(sizeof(float) * m_meta.feat_len)) {
            ESP_LOGE(TAG, "Failed to read feature data.");
            heap_caps_free(feat);
            mp_close(f);
            return ESP_FAIL;
        }

        // Füge das Feature zur internen Liste hinzu
        m_feats.emplace_back(id, feat);
    }

    // Überprüfe die Anzahl der gültigen Features
    if (m_feats.size() != m_meta.num_feats_valid) {
        ESP_LOGE(TAG, "Incorrect valid feature num.");
        mp_close(f);
        return ESP_FAIL;
    }

    // Schließe die Datei
    mp_close(f);
    return ESP_OK;
}

esp_err_t DataBase::enroll_feat(dl::TensorBase *feat)
{
    ESP_LOGI(TAG, "Enrolling feature.");
    if (feat->dtype != dl::DATA_TYPE_FLOAT) {
        ESP_LOGE(TAG, "Only support float feature.");
        return ESP_FAIL;
    }
    if (feat->size != m_meta.feat_len) {
        ESP_LOGE(TAG, "Feature len to enroll does not match feature len in db.");
        return ESP_FAIL;
    }

    // Kopiere das Feature in den Speicher
    float *feat_copy = (float *)heap_caps_malloc(m_meta.feat_len * sizeof(float), MALLOC_CAP_SPIRAM);
    memcpy(feat_copy, feat->data, feat->get_bytes());

    // Füge das Feature zur internen Liste hinzu
    m_feats.emplace_back(m_meta.num_feats_total + 1, feat_copy);
    m_meta.num_feats_total++;
    m_meta.num_feats_valid++;

    // Öffne die Datei mit `mp_open`
    mp_file_t *f = mp_open(m_db_path, "rb+");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open db.");
        return ESP_FAIL;
    }

    // Schreibe die Metadaten in die Datei
    mp_int_t size = mp_write(f, &m_meta, sizeof(mp_esp_dl::recognition::database_meta));
    if (size != sizeof(mp_esp_dl::recognition::database_meta)) {
        ESP_LOGE(TAG, "Failed to write database meta.");
        mp_close(f);
        return ESP_FAIL;
    }

    // Setze die Position ans Ende der Datei
    if (mp_seek(f, 0, SEEK_END) < 0) {
        ESP_LOGE(TAG, "Failed to seek db file.");
        mp_close(f);
        return ESP_FAIL;
    }

    // Schreibe die Feature-ID in die Datei
    size = mp_write(f, &m_feats.back().id, sizeof(uint16_t));
    if (size != sizeof(uint16_t)) {
        ESP_LOGE(TAG, "Failed to write feature id.");
        mp_close(f);
        return ESP_FAIL;
    }

    // Schreibe das Feature in die Datei
    size = mp_write(f, m_feats.back().feat, sizeof(float) * m_meta.feat_len);
    if (size != (mp_int_t)(sizeof(float) * m_meta.feat_len)) {
        ESP_LOGE(TAG, "Failed to write feature.");
        mp_close(f);
        return ESP_FAIL;
    }

    // Schließe die Datei
    mp_close(f);
    return ESP_OK;
}

esp_err_t DataBase::delete_feat(uint16_t id)
{
    bool invalid_id = true;

    // Entferne das Feature aus der internen Liste
    for (auto it = m_feats.begin(); it != m_feats.end();) {
        if (it->id != id) {
            it++;
        } else {
            heap_caps_free(it->feat);
            it = m_feats.erase(it);
            m_meta.num_feats_valid--;
            invalid_id = false;
            break;
        }
    }

    if (invalid_id) {
        ESP_LOGW(TAG, "Invalid id to delete.");
        return ESP_FAIL;
    }

    // Öffne die Datei mit `mp_open`
    mp_file_t *f = mp_open(m_db_path, "rb+");
    if (!f) {
        ESP_LOGE(TAG, "Failed to open db.");
        return ESP_FAIL;
    }

    // Berechne den Offset für die zu löschende ID
    off_t offset = sizeof(mp_esp_dl::recognition::database_meta) +
                   (sizeof(uint16_t) + sizeof(float) * m_meta.feat_len) * (id - 1);
    uint16_t id_invalid = 0;

    // Setze die Position auf den Offset
    if (mp_seek(f, offset, SEEK_SET) < 0) {
        ESP_LOGE(TAG, "Failed to seek db file.");
        mp_close(f);
        return ESP_FAIL;
    }

    // Schreibe die ungültige ID in die Datei
    mp_int_t size = mp_write(f, &id_invalid, sizeof(uint16_t));
    if (size != sizeof(uint16_t)) {
        ESP_LOGE(TAG, "Failed to write feature id.");
        mp_close(f);
        return ESP_FAIL;
    }

    // Aktualisiere die Anzahl der gültigen Features
    offset = sizeof(uint16_t);
    if (mp_seek(f, offset, SEEK_SET) < 0) {
        ESP_LOGE(TAG, "Failed to seek db file.");
        mp_close(f);
        return ESP_FAIL;
    }

    size = mp_write(f, &m_meta.num_feats_valid, sizeof(uint16_t));
    if (size != sizeof(uint16_t)) {
        ESP_LOGE(TAG, "Failed to write valid feature num.");
        mp_close(f);
        return ESP_FAIL;
    }

    // Schließe die Datei
    mp_close(f);
    return ESP_OK;
}

esp_err_t DataBase::delete_last_feat()
{
    if (m_feats.empty()) {
        ESP_LOGW(TAG, "Empty db, nothing to delete");
        return ESP_FAIL;
    }
    uint16_t id = m_feats.back().id;
    return delete_feat(id);
}

float DataBase::cal_similarity(float *feat1, float *feat2)
{
    float sum = 0;
    for (int i = 0; i < m_meta.feat_len; i++) {
        sum += feat1[i] * feat2[i];
    }
    return sum;
}

std::vector<mp_esp_dl::recognition::result_t> DataBase::query_feat(dl::TensorBase *feat, float thr, int top_k)
{
    if (top_k < 1) {
        ESP_LOGW(TAG, "Top_k should be greater than 0.");
        return {};
    }
    std::vector<mp_esp_dl::recognition::result_t> results;
    float sim;
    int i = 1;
    for (auto it = m_feats.begin(); it != m_feats.end(); it++, i++) {
        sim = cal_similarity(it->feat, (float *)feat->data);
        if (sim <= thr) {
            continue;
        }
        // results.emplace_back(it->id, sim);
        results.emplace_back(i, sim);
    }
    std::sort(results.begin(), results.end(), [](const mp_esp_dl::recognition::result_t &a, const mp_esp_dl::recognition::result_t &b) -> bool {
        return a.similarity > b.similarity;
    });
    if (results.size() > top_k) {
        results.resize(top_k);
    }
    return results;
}

void DataBase::print()
{
    mp_printf(&mp_plat_print, "\n");
    mp_printf(&mp_plat_print, "[db meta]\nnum_feats_total: %d, num_feats_valid: %d, feat_len: %d\n",
              m_meta.num_feats_total,
              m_meta.num_feats_valid,
              m_meta.feat_len);
    // mp_printf(&mp_plat_print, "[feats]\n");
    // for (auto it : m_feats) {
    //     mp_printf(&mp_plat_print, "id: %d feat: ", it.id);
    //     for (int i = 0; i < m_meta.feat_len; i++) {
    //         mp_printf(&mp_plat_print, "%f, ", it.feat[i]);
    //     }
    //     mp_printf(&mp_plat_print, "\n");
    // }
    mp_printf(&mp_plat_print, "\n");
}

} // namespace recognition
} // namespace mp_esp_dl