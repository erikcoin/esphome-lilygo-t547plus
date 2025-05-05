#include "papers33.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/color.h"
#include <cmath>
#include <algorithm> // For std::min and std::max
#include <esp_heap_caps.h> // Include for heap info functions

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

// Helper function to convert ESPHome Color to a 4-bit grayscale palette index (0-15)
static inline uint8_t get_grayscale_palette_index(esphome::Color color) {
    float gray_value = (color.r + color.g + color.b) / 3.0f;
    float index_float = (255.0f - gray_value) / 255.0f * 15.0f;
    uint8_t index = static_cast<uint8_t>(roundf(index_float));
    index = std::min((uint8_t)15, std::max((uint8_t)0, index));
    return index;
}


void M5PaperS3DisplayM5GFX::setup() {
    ESP_LOGD(TAG, "Memory before M5.begin():");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Total PSRAM: %u bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));


    ESP_LOGD(TAG, "Calling M5.config()...");
    auto cfg = M5.config();
   // //cfg.use_psram = true; // Ensure this is true
   // ESP_LOGD(TAG, "M5.config().use_psram set to: %s", cfg.use_psram ? "true" : "false");
    ESP_LOGD(TAG, "Calling M5.begin()...");

    M5.begin(cfg); // This is where I2C errors are seen in logs
    ESP_LOGD(TAG, "M5.begin() finished.");

    // --- Add a delay immediately after M5.begin() ---
    ESP_LOGD(TAG, "Adding delay after M5.begin()...");
    delay(100); // Small delay after M5.begin()
    ESP_LOGD(TAG, "Delay finished.");
    // --------------------------------------------------


    ESP_LOGD(TAG, "Memory after M5.begin() + delay:");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));


    ESP_LOGD(TAG, "Calling M5.Display.setEpdMode()...");
    M5.Display.setEpdMode(epd_mode_t::epd_quality);
    ESP_LOGD(TAG, "M5.Display.setEpdMode() finished.");

    ESP_LOGD(TAG, "Waiting for EPD to be readable...");
    while (!M5.Display.isReadable()) {
        ESP_LOGD(TAG, "EPD not readable yet, waiting...");
        delay(1000);
    }
    ESP_LOGD(TAG, "EPD is readable.");

    ESP_LOGD(TAG, "Adding delay after readable before touch/sprite creation...");
    delay(500);
    ESP_LOGD(TAG, "Delay finished.");

    // Touch block commented out as per user's testing
    
    if (this->touch_coordinates_sensor_ != nullptr) {
        ESP_LOGD(TAG, "Touch sensor configured, entering touch block...");
        ESP_LOGD(TAG, "Publishing test value to touch sensor...");
        this->touch_coordinates_sensor_->publish_state("42,84");
        ESP_LOGD(TAG, "Test value published.");
        ESP_LOGD(TAG, "Exiting touch block.");
    } else {
        ESP_LOGD(TAG, "Touch sensor not configured, skipping touch block.");
    }
    

    auto &gfx = M5.Display;
    // Rotation code removed by user


    if (this->canvas_ != nullptr) {
      ESP_LOGD(TAG, "Deleting existing canvas_...");
      delete this->canvas_;
      this->canvas_ = nullptr;
      ESP_LOGD(TAG, "Existing canvas_ deleted.");
    }

    // --- Add a delay specifically before getting display dimensions ---
    ESP_LOGD(TAG, "Adding delay before getting display dimensions...");
    delay(100); // Delay before calling width()/height()
    ESP_LOGD(TAG, "Delay finished.");
    // -----------------------------------------------------------------

    // Get display dimensions using gfx
    // *** CRASH LIKELY HAPPENS HERE WHEN CALLING gfx.width() or gfx.height() ***
    ESP_LOGD(TAG, "About to call gfx.width() and gfx.height()..."); // New log before the call
    int display_width = gfx.width(); // Get dimensions BEFORE allocation test
    int display_height = gfx.height(); // Get dimensions BEFORE allocation test
    ESP_LOGD(TAG, "Finished calling gfx.width() and gfx.height(). Display dimensions: %d x %d", display_width, display_height); // Log after and get values again


    // Calculate required memory for the sprite buffer (based on retrieved dimensions)
    size_t required_bytes = (size_t)display_width * display_height * 4 / 8; // Use retrieved dimensions
    ESP_LOGD(TAG, "Estimated memory needed for sprite buffer (4 bit): %u bytes", required_bytes);


     // Log memory status before new LGFX_Sprite
    ESP_LOGD(TAG, "Memory before new LGFX_Sprite:");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));

    ESP_LOGD(TAG, "Calling new lgfx::v1::LGFX_Sprite(&gfx)...");
    this->canvas_ = new lgfx::v1::LGFX_Sprite(&gfx);
    ESP_LOGD(TAG, "new lgfx::v1::LGFX_Sprite() finished.");
    this->canvas_->setPsram(true);  // <-- DIT IS BELANGRIJK

    if (this->canvas_ == nullptr) {
         ESP_LOGE(TAG, "Failed to create LGFX_Sprite object itself (out of internal RAM?)!");
         return;
    }

    ESP_LOGD(TAG, "Setting canvas color depth to 4 bits...");
    this->canvas_->setColorDepth(4);
    ESP_LOGD(TAG, "Canvas color depth set.");

    ESP_LOGD(TAG, "Setting canvas palette to grayscale...");
    this->canvas_->setPaletteGrayscale();
    ESP_LOGD(TAG, "Canvas palette set.");

     // Log memory status before createSprite (actual allocation)
    ESP_LOGD(TAG, "Memory before canvas_->createSprite():");
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "Calling canvas_->createSprite(%d, %d)...", display_width, display_height); // Log dimensions used

    // This is the call that allocates the main pixel buffer
    bool ok = this->canvas_->createSprite(display_width, display_height); // Use stored dimensions
    ESP_LOGD(TAG, "canvas_->createSprite() finished. Result: %s", ok ? "true" : "false");


    if (!ok) {
        ESP_LOGE(TAG, "Failed to create canvas sprite buffer! Check memory (PSRAM) and fragmentation.");
        ESP_LOGE(TAG, "Sprite allocation requested size: %d x %d", display_width, display_height); // Log dimensions requested
        // Log memory status after failed createSprite
        ESP_LOGE(TAG, "Memory after *failed* createSprite:");
        ESP_LOGE(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGE(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

        // Cleanup the sprite object itself if buffer allocation failed
        delete this->canvas_;
        this->canvas_ = nullptr;
    } else {
        ESP_LOGD(TAG, "Canvas sprite buffer created successfully.");
        ESP_LOGD(TAG, "Sprite allocated with size: %d x %d", this->canvas_->width(), this->canvas_->height());
        // Log memory status after successful createSprite
        ESP_LOGD(TAG, "Memory after *successful* createSprite:");
        ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

        //ESP_LOGD(TAG, "Calling canvas_->fillSprite()...");
        //this->canvas_->fillSprite(0);
        //ESP_LOGD(TAG, "canvas_->fillSprite() finished.");
    }

    if (!M5.Touch.isEnabled()) {
        ESP_LOGW(TAG, "Touchscreen not enabled or GT911 not found.");
    } else {
        ESP_LOGI(TAG, "Touchscreen initialized.");
    }
     ESP_LOGD(TAG, "End of setup().");

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

    //M5.Display.setEpdMode(epd_mode_t::epd_quality);

    if (this->writer_ != nullptr) {
        ESP_LOGD(TAG, "Clearing canvas sprite (fill with index 0 = white)");
        this->canvas_->fillSprite(15);

        ESP_LOGD(TAG, "Calling writer lambda...");
        this->writer_(*this);

        ESP_LOGD(TAG, "Pushing sprite to display buffer...");
        this->canvas_->pushSprite(0, 0);

        ESP_LOGD(TAG, "Triggering EPD refresh (display)...");
        //this->gfx_.display();
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
    ESP_LOGD(TAG, "drawPixel  with color: %d ", palette_index);
    this->canvas_->drawPixel(x, y, palette_index);
}

void M5PaperS3DisplayM5GFX::set_writer(std::function<void(display::Display &)> writer) {
  this->writer_ = writer;
}

} // namespace m5papers3_display_m5gfx
} // namespace esphome
