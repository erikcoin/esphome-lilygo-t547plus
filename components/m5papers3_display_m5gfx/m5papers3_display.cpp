#include "m5papers3_display.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

void M5PaperS3DisplayM5GFX::setup() {
    ESP_LOGD(TAG, "Free heap: %d bytes", esp_get_free_heap_size());

    auto cfg = M5.config();
    M5.begin(cfg);

    ESP_LOGD(TAG, "M5.begin() finished.");
    M5.Display.setEpdMode(epd_mode_t::epd_quality);
    while (!M5.Display.isReadable()) {
        ESP_LOGD(TAG, "Waiting for EPD to be ready...");
        delay(1000);
    }

    M5.Display.clearDisplay();
    auto &gfx = M5.Display;
    gfx.setRotation(this->rotation_);
    ESP_LOGD(TAG, "M5GFX Rotation set to: %d", this->rotation_);
    
    gfx.fillScreen(TFT_WHITE);
    gfx.display();
    gfx.waitDisplay();

    // Setup canvas
    ESP_LOGD(TAG, "Creating canvas...");
    this->canvas_ = M5Canvas(&gfx);
    this->canvas_.setColorDepth(4);  // 4 bits: 16 grijstinten

    bool ok = this->canvas_.createSprite(gfx.width(), gfx.height());
    if (!ok) {
        ESP_LOGE(TAG, "Failed to create canvas sprite!");
    } else {
        ESP_LOGD(TAG, "Canvas created with size: %d x %d", gfx.width(), gfx.height());
    }
}

void M5PaperS3DisplayM5GFX::update() {
    static bool first_time = true;
    if (first_time) {
        ESP_LOGD(TAG, "Delaying first update for EPD...");
        delay(1000);
        first_time = false;
    }
    ESP_LOGD(TAG, "Running M5GFX display update...");

    M5.Display.setEpdMode(epd_mode_t::epd_quality);
    if (this->writer_ != nullptr) {
        ESP_LOGD(TAG, "Clearing canvas to white...");
        this->canvas_.fillSprite(15);  // 15 = wit in 4bpp

        this->canvas_.setTextColor(0); // 0 = zwart
        ESP_LOGD(TAG, "Starting writer lambda...");
        this->writer_(*this);  

        ESP_LOGD(TAG, "Writer done, pushing canvas...");
        this->canvas_.pushSprite(0, 0);

        M5.Display.display();                 
        M5.Display.waitDisplay();
    }
}

void M5PaperS3DisplayM5GFX::dump_config() {
    LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper", this);
    ESP_LOGCONFIG(TAG, "  Rotation: %d degrees", this->rotation_ * 90);
    LOG_UPDATE_INTERVAL(this);
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
    this->draw_absolute_pixel_internal(x, y, color);
}

void M5PaperS3DisplayM5GFX::set_rotation(int rotation) {
    int m5gfx_rotation = 0;
    switch (rotation) {
        case 90: m5gfx_rotation = 1; break;
        case 180: m5gfx_rotation = 2; break;
        case 270: m5gfx_rotation = 3; break;
        default: m5gfx_rotation = 0; break;
    }
    this->rotation_ = m5gfx_rotation;
}

int M5PaperS3DisplayM5GFX::get_width_internal() {
    return M5.Display.width();
}

int M5PaperS3DisplayM5GFX::get_height_internal() {
    return M5.Display.height();
}

void M5PaperS3DisplayM5GFX::fill(Color color) {
    ESP_LOGD(TAG, "Canvas fill called");
    uint8_t gray = get_native_m5gfx_color_(color);
    this->canvas_.fillSprite(gray);
}

void M5PaperS3DisplayM5GFX::draw_absolute_pixel_internal(int x, int y, Color color) {
    if (x < 0 || x >= this->get_width_internal() || y < 0 || y >= this->get_height_internal())
        return;
    uint8_t gray = this->get_native_m5gfx_color_(color);
    this->canvas_.drawPixel(x, y, gray);
}

// Zet esphome kleur om naar 4-bit grijswaarde (0-15)
uint8_t M5PaperS3DisplayM5GFX::get_native_m5gfx_color_(Color color) {
    // Gebruik echte luminantie correctie
    float brightness = (0.299f * color.r + 0.587f * color.g + 0.114f * color.b) / 255.0f;
    uint8_t gray = static_cast<uint8_t>(brightness * 15.0f);  
    return gray;
}

} // namespace m5papers3_display_m5gfx
} // namespace esphome
