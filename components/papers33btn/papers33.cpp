#include "papers33.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/color.h"
#include <cmath>
#include <string>
#include <algorithm> // For std::min and std::max
#include <esp_heap_caps.h> // Include for heap info functions

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

// Helper function to convert ESPHome Color to a 4-bit grayscale palette index (0-15)
//static inline uint8_t get_grayscale_palette_index(esphome::Color color) {
//    float gray_value = (color.r + color.g + color.b) / 3.0f;
//    float index_float = (255.0f - gray_value) / 255.0f * 15.0f;
//    uint8_t index = static_cast<uint8_t>(roundf(index_float));
//    index = std::min((uint8_t)15, std::max((uint8_t)0, index));
//    return index;
//}

void M5PaperS3DisplayM5GFX::setup() {
  ESP_LOGI(TAG, "Initializing M5PaperS3DisplayM5GFX");
  this->gfx_.begin();
  this->gfx_.setRotation(0);
  this->gfx_.setEpdMode(epd_mode_t::epd_fastest);

  for (int i = 0; i < 6; i++) {
    button_sprites_[i] = new LGFX_Sprite(&gfx_);
    button_sprites_[i]->setPsram(true);
    button_sprites_[i]->createSprite(BUTTON_WIDTH, BUTTON_HEIGHT);

    draw_button_(i, "Btn " + std::to_string (i + 1), TFT_WHITE, TFT_BLACK);

    int row = i / 2;
    int col = i % 2;
    int x = col * BUTTON_WIDTH;
    int y = row * BUTTON_HEIGHT;
    button_sprites_[i]->pushSprite(x, y);
  }

  this->gfx_.display();  // Refresh e-paper
}


void M5PaperS3DisplayM5GFX::update() {
    static bool first_time = true;
    if (first_time) {
        ESP_LOGD(TAG, "Delaying first update for EPD...");
        delay(1000);
        first_time = false;
    }

    if (this->canvas_ == nullptr) {
        ESP_LOGE(TAG, "Canvas not available in update()!");
        return;
    }
M5.Display.setEpdMode(epd_mode_t::epd_fastest); // Perform a full hardware reset
M5.Display.display();
vTaskDelay(pdMS_TO_TICKS(1000)); // Give enough time for full refresh

ESP_LOGD(TAG, "Clearing ghost artifacts using fast refresh mode...");
for (int i = 0; i < 3; i++) {
    M5.Display.fillScreen(0xAAAAAA);  // Medium gray
    M5.Display.display();
    vTaskDelay(pdMS_TO_TICKS(500));

    M5.Display.fillScreen(0x555555);  // Darker gray
    M5.Display.display();
    vTaskDelay(pdMS_TO_TICKS(500));

    M5.Display.fillScreen(0xFFFFFF);  // Full white
    M5.Display.display();
    vTaskDelay(pdMS_TO_TICKS(500));
}

    if (this->writer_ != nullptr) {
        ESP_LOGD(TAG, "Clearing canvas sprite (fill with index 0 = white)");
        this->canvas_->fillSprite(15);

        ESP_LOGD(TAG, "Calling writer lambda...");
        this->writer_(*this);

        ESP_LOGD(TAG, "Pushing sprite to display buffer...");
        this->canvas_->pushSprite(0, 0);

        ESP_LOGD(TAG, "Triggering EPD refresh (display)...");
        this->gfx_.display();
    } else {
        ESP_LOGD(TAG, "No writer lambda set, skipping drawing.");
    }

    ESP_LOGD(TAG, "EPD refresh completed.");
}

void M5PaperS3DisplayM5GFX::update_button(int index, const std::string &label, uint16_t bg_color, uint16_t text_color) {
  if (index < 0 || index >= 6) return;

  auto* spr = button_sprites_[index];
  if (!spr) return;

  spr->fillScreen(bg_color);
  spr->setTextColor(text_color);
  spr->setTextDatum(middle_center);
  spr->drawString(label.c_str(), BUTTON_WIDTH / 2, BUTTON_HEIGHT / 2);

  int x = (index % 2) * BUTTON_WIDTH;
  int y = (index / 2) * BUTTON_HEIGHT;
  spr->pushSprite(x, y);

  this->gfx_.display();  // Optional: only for e-paper
}

// Touch related functions (no changes needed here from previous correction)
bool M5PaperS3DisplayM5GFX::get_touch(TouchPoint *point) {
    m5::touch_point_t tp[1];
    int touch = M5.Display.getTouchRaw(tp, 1);
    if (touch > 0) {
        point->x = tp[0].x;
        point->y = tp[0].y;
        return true;
    }
    return false;
}
void M5PaperS3DisplayM5GFX::draw_button_(int index, const char* label, uint16_t bg_color, uint16_t text_color) {
  if (!button_sprites_[index]) return;

  auto* spr = button_sprites_[index];
  spr->fillScreen(bg_color);
  spr->setTextColor(text_color);
  spr->setTextDatum(middle_center);
  spr->drawString(label, BUTTON_WIDTH / 2, BUTTON_HEIGHT / 2);
}




void M5PaperS3DisplayM5GFX::update_touch() {
  TouchPoint point;
  if (!this->get_touch(&point)) {
    return;
  }

  if (this->touch_coordinates_sensor_ == nullptr) {
    return;
  }

  this->send_coordinates(point);
}

void M5PaperS3DisplayM5GFX::send_coordinates(TouchPoint tp) {
  if (this->touch_coordinates_sensor_ != nullptr) {
    std::string coords = std::to_string(tp.x) + "," + std::to_string(tp.y);
    this->touch_coordinates_sensor_->publish_state(coords);

    this->set_timeout("clear_touch_sensor", 200, [this]() {
      if (this->touch_coordinates_sensor_ != nullptr)
        this->touch_coordinates_sensor_->publish_state("");
    });
  }
}

void M5PaperS3DisplayM5GFX::set_touch_sensor(text_sensor::TextSensor *touch_coordinates_sensor) {
  ESP_LOGD(TAG, "Setting touch_coordinates_sensor...");
  this->touch_coordinates_sensor_ = touch_coordinates_sensor;
  ESP_LOGD(TAG, "Touch_coordinates_sensor is set");

  this->set_interval("touch_poll", 100, [this]() { this->update_touch(); });
}


void M5PaperS3DisplayM5GFX::dump_config() {
    LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper (4-bit Grayscale)", this);
    int display_rotation_deg = this->rotation_ * 90;
    ESP_LOGCONFIG(TAG, "  Rotation: %d degrees", display_rotation_deg);
    if (this->canvas_) {
        ESP_LOGCONFIG(TAG, "  Canvas Size: %d x %d", this->canvas_->width(), this->canvas_->height());
        ESP_LOGCONFIG(TAG, "  Canvas Color Depth: %d bits", this->canvas_->getColorDepth());
    } else {
        ESP_LOGCONFIG(TAG, "  Canvas: Not Initialized");
    }
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
        default: m5gfx_rotation = 0; break; // Case 0 and others map to 0
    }
    this->rotation_ = m5gfx_rotation;
}

int M5PaperS3DisplayM5GFX::get_width_internal() {
    // Note: M5GFX width/height changes based on rotation.
    return (this->canvas_) ? this->canvas_->width() : M5.Display.width();
}

int M5PaperS3DisplayM5GFX::get_height_internal() {
    // Note: M5GFX width/height changes based on rotation.
    return (this->canvas_) ? this->canvas_->height() : M5.Display.height();
}

//void M5PaperS3DisplayM5GFX::fill(esphome::Color color) {
//    if (this->canvas_ == nullptr) return;
//    uint8_t palette_index = get_grayscale_palette_index(color);
//    ESP_LOGD(TAG, "fillsprite with color: %d ", palette_index);
//    this->canvas_->fillSprite(palette_index);
//}

//void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
//    if (this->canvas_ == nullptr) return;
//    if (x < 0 || x >= this->get_width_internal() || y < 0 || y >= this->get_height_internal()) {
//        return;
//    }
//    uint8_t palette_index = get_grayscale_palette_index(color);
//    //ESP_LOGD(TAG, "drawPixel  with color: %d ", palette_index);
//    this->canvas_->drawPixel(x, y, palette_index);
//}
void M5PaperS3DisplayM5GFX::fill(Color color) /* override */ {
    if (this->canvas_ == nullptr) return;
    // Convert ESPHome Color (r,g,b are 0-255) to RGB888 format (uint32_t)
    // using the M5GFX helper function available via our gfx_ reference.
    uint32_t rgb888_color = this->gfx_.color888(color.r, color.g, color.b);

    ESP_LOGV(TAG, "fill() called with RGB888: %x", rgb888_color);

    // Pass the full color value to fillSprite.
    // The sprite will find the closest match in its active grayscale palette.
    this->canvas_->fillSprite(rgb888_color);
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) /* override */ {
    if (this->canvas_ == nullptr) return;
    // Check bounds using canvas dimensions
    if (x < 0 || x >= this->canvas_->width() || y < 0 || y >= this->canvas_->height()) {
        return;
    }
    // Convert ESPHome Color to RGB888 format
    uint32_t rgb888_color = this->gfx_.color888(color.r, color.g, color.b);

    // Pass the full color value to drawPixel.
    // The sprite will find the closest match in its active grayscale palette.
    this->canvas_->drawPixel(x, y, rgb888_color);
}



void M5PaperS3DisplayM5GFX::set_writer(std::function<void(display::Display &)> writer) {
  this->writer_ = writer;
}

} // namespace m5papers3_display_m5gfx
} // namespace esphome
