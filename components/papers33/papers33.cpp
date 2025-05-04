#include "papers33.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/color.h"
#include <cmath>
#include <algorithm> // For std::min and std::max

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

// Helper function to convert ESPHome Color to a 4-bit grayscale palette index (0-15)
// Assumes palette index 0 is white and 15 is black.
static inline uint8_t get_grayscale_palette_index(esphome::Color color) {
    // Calculate a simple grayscale value (0-255)
    // Use average of R, G, B components
    float gray_value = (color.r + color.g + color.b) / 3.0f;

    // Map gray_value [0, 255] to palette index [15, 0]
    // gray_value 0 (black) should map to index 15
    // gray_value 255 (white) should map to index 0
    float index_float = (255.0f - gray_value) / 255.0f * 15.0f;
    uint8_t index = static_cast<uint8_t>(roundf(index_float));

    // Ensure index is within bounds [0, 15]
    index = std::min((uint8_t)15, std::max((uint8_t)0, index));

    return index;
}


void M5PaperS3DisplayM5GFX::setup() {
    // Log memory status before M5.begin()
    ESP_LOGD(TAG, "Memory before M5.begin():");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Total PSRAM: %u bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));


    auto cfg = M5.config();
    //cfg.use_psram = true; // Ensure this is true
   // ESP_LOGD(TAG, "M5.config().use_psram set to: %s", cfg.use_psram ? "true" : "false");

    M5.begin(cfg);

    ESP_LOGD(TAG, "M5.begin() finished.");
    // Log memory status after M5.begin()
    ESP_LOGD(TAG, "Memory after M5.begin():");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));


    M5.Display.setEpdMode(epd_mode_t::epd_quality);

    while (!M5.Display.isReadable()) {
        ESP_LOGD(TAG, "Waiting for EPD to be ready...");
        delay(1000);
    }

    if (this->touch_coordinates_sensor_ != nullptr) {
        ESP_LOGD(TAG, "Publishing test value to touch sensor.");
        this->touch_coordinates_sensor_->publish_state("42,84");
    }

    auto &gfx = M5.Display;
    //gfx.setRotation(this->rotation_);
    ESP_LOGD(TAG, "M5GFX Rotation set to: %d", this->rotation_);

    if (this->canvas_ != nullptr) {
      delete this->canvas_;
    }

    // Calculate required memory for the sprite buffer
    size_t required_bytes = (size_t)gfx.width() * gfx.height() * this->canvas_->getColorDepth() / 8;
    ESP_LOGD(TAG, "Attempting to create canvas sprite %d x %d @ %d bits", gfx.width(), gfx.height(), this->canvas_->getColorDepth());
    ESP_LOGD(TAG, "Estimated memory needed for sprite: %u bytes", required_bytes);
     // Log memory status before createSprite
    ESP_LOGD(TAG, "Memory before createSprite:");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));


    // Use the fully qualified name for LGFX_Sprite
    // The new call allocates the sprite object itself, but createSprite allocates the buffer
    this->canvas_ = new lgfx::v1::LGFX_Sprite(&gfx);

    // After 'new' but before 'createSprite', canvas_ is non-null but the buffer is not allocated
    if (this->canvas_ == nullptr) {
         ESP_LOGE(TAG, "Failed to create LGFX_Sprite object itself!");
         // This is a different error than createSprite failing, likely internal RAM issue
         return; // Cannot proceed if sprite object fails
    }


    // This is the call that allocates the main pixel buffer, likely in PSRAM
    bool ok = this->canvas_->createSprite(gfx.width(), gfx.height());
    if (!ok) {
        ESP_LOGE(TAG, "Failed to create canvas sprite buffer! Check memory (try enabling PSRAM and checking fragmentation?).");
        // Log memory status after failed createSprite
        ESP_LOGE(TAG, "Memory after *failed* createSprite:");
        ESP_LOGE(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
        ESP_LOGE(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGE(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
        ESP_LOGE(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

        delete this->canvas_;
        this->canvas_ = nullptr; // Set to nullptr so update() knows it failed
    } else {
        ESP_LOGD(TAG, "Canvas sprite buffer created successfully.");
        // Log memory status after successful createSprite
        ESP_LOGD(TAG, "Memory after *successful* createSprite:");
        ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
        ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGD(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
        ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

        this->canvas_->fillSprite(0);
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

    if (this->canvas_ == nullptr) {
        ESP_LOGE(TAG, "Canvas not available in update()!");
        return;
    }

    M5.Display.setEpdMode(epd_mode_t::epd_quality);

    if (this->writer_ != nullptr) {
        ESP_LOGD(TAG, "Clearing canvas sprite (fill with index 0 = white)");
        this->canvas_->fillSprite(0);

        ESP_LOGD(TAG, "Calling writer lambda...");
        this->writer_(*this);

        ESP_LOGD(TAG, "Pushing sprite to display buffer...");
        this->canvas_->pushSprite(0, 0);

        ESP_LOGD(TAG, "Triggering EPD refresh (display)...");
        this->gfx_.display();
    } else {
        ESP_LOGD(TAG, "No writer lambda set, skipping drawing.");
    }

    update_touch();
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

void M5PaperS3DisplayM5GFX::fill(Color color) {
    if (this->canvas_ == nullptr) return;
    uint8_t palette_index = get_grayscale_palette_index(color);
    this->canvas_->fillSprite(palette_index);
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
    if (this->canvas_ == nullptr) return;
    if (x < 0 || x >= this->get_width_internal() || y < 0 || y >= this->get_height_internal()) {
        return;
    }
    uint8_t palette_index = get_grayscale_palette_index(color);
    this->canvas_->drawPixel(x, y, palette_index);
}

void M5PaperS3DisplayM5GFX::set_writer(std::function<void(display::Display &)> writer) {
  this->writer_ = writer;
}

} // namespace m5papers3_display_m5gfx
} // namespace esphome
