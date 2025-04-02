#include "mp_esp_dl.hpp"

namespace mp_esp_dl {
    void initialize_img(dl::image::img_t &img, int width, int height, dl::image::pix_type_t pix_type) {
        img.width = width;
        img.height = height;
        img.pix_type = pix_type;
        img.data = nullptr;
    }
} // namespace mp_esp_dl