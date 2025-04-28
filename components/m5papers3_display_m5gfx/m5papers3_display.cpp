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
  // Stel een interval in om elke 100 ms de update_touch() functie aan te roepen
  this->set_interval(100, [this]() { 
    this->update_touch();
  });
    ESP_LOGD(TAG, "M5.begin() finished.");
    M5.Display.setEpdMode(epd_mode_t::epd_fastest);
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

    if (this->canvas_ != nullptr) {
      delete this->canvas_;
    }
    this->canvas_ = new lgfx::LGFX_Sprite(&gfx);
    this->canvas_->setColorDepth(1);
    bool ok = this->canvas_->createSprite(gfx.width(), gfx.height());
    if (!ok) {
        ESP_LOGE(TAG, "Failed to create canvas sprite!");
    } else {
        ESP_LOGD(TAG, "Canvas created with size: %d x %d", gfx.width(), gfx.height());
    }

    if (!M5.Touch.isEnabled()) {
        ESP_LOGW(TAG, "Touchscreen not enabled or GT911 not found.");
    } else {
        ESP_LOGI(TAG, "Touchscreen initialized.");
    }
}

void M5PaperS3DisplayM5GFX::update() {
    static bool first_time = true;
    if (first_time) {
        ESP_LOGD(TAG, "Delaying first update for EPD...");
        delay(1000);
        first_time = false;
    }

    M5.Display.setEpdMode(epd_mode_t::epd_fastest);
    if (this->writer_ != nullptr) {
        this->canvas_->fillSprite(TFT_WHITE);
        this->writer_(*this);
        this->canvas_->pushSprite(0, 0);
    }

    update_touch();
}

// ======= Touch gerelateerde functies =======

bool M5PaperS3DisplayM5GFX::get_touch(TouchPoint *point) {
    m5::touch_point_t tp[1];  // Gebruik m5::touch_point_t
    int touch = M5.Display.getTouchRaw(tp, 1);
    if (touch > 0) {
        ESP_LOGD(TAG, "Raw touch detected at (%d, %d)", tp[0].x, tp[0].y);
        point->x = tp[0].x;
        point->y = tp[0].y;
        return true;
    }
    ESP_LOGD(TAG, "No raw touch detected.");
    return false;
}

void M5PaperS3DisplayM5GFX::handle_touch(uint16_t x, uint16_t y) {
    ESP_LOGI(TAG, "Handling touch at (%d, %d)", x, y);
    M5.Lcd.fillCircle(x, y, 10, GREEN);
}

// ===========================================

void M5PaperS3DisplayM5GFX::dump_config() {
    LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper", this);
    ESP_LOGCONFIG(TAG, "  Rotation: %d degrees", this->rotation_ * 90);
}

M5PaperS3DisplayM5GFX::~M5PaperS3DisplayM5GFX() {
  if (this->canvas_ != nullptr) {
    delete this->canvas_;
    this->canvas_ = nullptr;
    ESP_LOGD(TAG, "Canvas deleted safely.");
  }
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
    uint16_t col = color.is_on() ? TFT_BLACK : TFT_WHITE;
    this->canvas_->fillSprite(col);
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
    uint16_t col = color.is_on() ? TFT_BLACK : TFT_WHITE;
    this->canvas_->drawPixel(x, y, col);
}


void M5PaperS3DisplayM5GFX::update_touch() {
    ESP_LOGD(TAG, "Checking for touch...");
    if (this->get_touch(&touch_point_)) {
        ESP_LOGD(TAG, "Touch detected at (%d, %d)", touch_point_.x, touch_point_.y);
        handle_touch(touch_point_.x, touch_point_.y);
    } else {
        ESP_LOGD(TAG, "No touch detected.");
    }
}

//void M5PaperS3DisplayM5GFX::update_touch() {
//    ESP_LOGD(TAG, "update touch %d %d", tp[0].x, tp[0].y);
//  m5::touch_point_t tp[1];
//  if (M5.Display.getTouchRaw(tp, 1) > 0) {
//    this->touch_detected_ = true;
 //   this->touch_x_ = tp[0].x;
//   this->touch_y_ = tp[0].y;
 //   handle_touch(tp[0].x, tp[0].y);
      
 // } else {
 //   this->touch_detected_ = false;
 // }
//}

void M5PaperS3DisplayM5GFX::set_writer(std::function<void(display::Display &)> writer) {
  this->writer_ = writer;
}
} // namespace m5papers3_display_m5gfx
} // namespace esphome
