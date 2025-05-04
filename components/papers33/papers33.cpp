#include "papers33.h" // Use the corrected header
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/color.h"
#include <cmath>

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

// Helper function (remains the same)
static inline uint8_t get_grayscale_palette_index(esphome::Color color) {
    float brightness = (0.299f * color.r + 0.587f * color.g + 0.114f * color.b) / 255.0f;
    uint8_t index = static_cast<uint8_t>(roundf((1.0f - brightness) * 15.0f));
    return index;
}


void M5PaperS3DisplayM5GFX::setup() /* override */ { // Add override comment/keyword if style requires
    ESP_LOGD(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    auto cfg = M5.config();
    M5.begin(cfg);
    ESP_LOGD(TAG, "M5.begin() finished.");

    // Use the gfx_ member reference now
    gfx.setEpdMode(epd_mode_t::epd_quality);
    ESP_LOGD(TAG, "EPD Mode set to Quality");

    while (!gfx.isReadable()) {
        ESP_LOGD(TAG, "Waiting for EPD to be ready...");
        delay(1000);
    }

    if (this->touch_coordinates_sensor_ != nullptr) {
        ESP_LOGD(TAG, "Publishing test value to touch sensor.");
        this->touch_coordinates_sensor_->publish_state("42,84");
    }

    gfx.setRotation(this->rotation_);
    ESP_LOGD(TAG, "M5GFX Rotation set to: %d", this->rotation_);


    if (this->canvas_ != nullptr) {
      delete this->canvas_;
    }
    // Create sprite using the gfx_ reference
     auto &gfx = M5.Display;
    this->canvas_ = new lgfx::LGFX_Sprite(gfx);
    //this->canvas_ = new lgfx::LGFX_Sprite(&this->gfx_);

    this->canvas_->setColorDepth(4);
    ESP_LOGD(TAG, "Canvas color depth set to 4 bits.");

    this->canvas_->setPaletteGrayscale();
    ESP_LOGD(TAG, "Canvas palette set to grayscale.");

    // Create the sprite buffer in memory
    bool ok = this->canvas_->createSprite(gfx.width(), gfx.height());
    if (!ok) {
        ESP_LOGE(TAG, "Failed to create canvas sprite! Check memory.");
        delete this->canvas_;
        this->canvas_ = nullptr;
    } else {
        ESP_LOGD(TAG, "Canvas created with size: %d x %d", this->canvas_->width(), this->canvas_->height());
        this->canvas_->fillSprite(0); // Initial clear to white (index 0)
    }

    // Check touch AFTER M5.begin() which initializes peripherals
    if (!M5.Touch.isEnabled()) {
         ESP_LOGW(TAG, "Touchscreen not enabled or GT911 not found.");
    } else {
         ESP_LOGI(TAG, "Touchscreen initialized.");
         // Setup touch polling interval ONLY if touch is enabled and sensor is set
         if (this->touch_coordinates_sensor_ != nullptr) {
             this->set_interval("touch_poll", 100, [this]() { this->update_touch(); });
             ESP_LOGI(TAG, "Touch polling interval started.");
         }
    }
}

void M5PaperS3DisplayM5GFX::update() /* override */ {
    // ... (static bool first_time logic can likely be removed if wait in setup is sufficient)

    if (this->canvas_ == nullptr) {
        ESP_LOGE(TAG, "Canvas not available in update()!");
        return;
    }

    // Ensure EPD mode is quality for the update operation
    gfx.setEpdMode(epd_mode_t::epd_quality);

    if (this->writer_ != nullptr) {
        ESP_LOGV(TAG, "Clearing canvas sprite (fill with index 0 = white)"); // Verbose log level
        this->canvas_->fillSprite(0);

        ESP_LOGV(TAG, "Calling writer lambda...");
        this->writer_(*this); // Pass 'this' which is a display::Display&

        ESP_LOGV(TAG, "Pushing sprite to display buffer...");
        this->canvas_->pushSprite(0, 0);

        ESP_LOGV(TAG, "Triggering EPD refresh (display)...");
        //this->gfx_.display(); // Trigger physical update

    } else {
        ESP_LOGD(TAG, "No writer lambda set, skipping drawing.");
    }

    // Note: Touch update is handled by the interval timer set in setup()
    // update_touch(); // Remove this call from here
}

// ======= Touch Functions =======
bool M5PaperS3DisplayM5GFX::get_touch(TouchPoint *point) {
    m5::touch_point_t tp[1];
    // Use gfx_ reference to get touch data associated with the display instance
    int touch = gfx.getTouchRaw(tp, 1);
    if (touch > 0) {
        point->x = tp[0].x;
        point->y = tp[0].y;
        return true;
    }
    return false;
}

void M5PaperS3DisplayM5GFX::update_touch() {
  // This is called periodically by the interval timer
  TouchPoint point; // Use local variable
  if (!this->get_touch(&point)) {
    return; // No touch detected this interval
  }

  // ESP_LOGD(TAG, "Raw touch detected via interval at (%d, %d)", point.x, point.y);

  if (this->touch_coordinates_sensor_ == nullptr) {
    return; // No sensor configured
  }

  // Send coordinates immediately when touch is detected
  this->send_coordinates(point);
}


void M5PaperS3DisplayM5GFX::send_coordinates(TouchPoint tp) {
  // This function is called by update_touch when a touch occurs
  if (this->touch_coordinates_sensor_ != nullptr) {
    std::string coords = std::to_string(tp.x) + "," + std::to_string(tp.y);
    this->touch_coordinates_sensor_->publish_state(coords);
    ESP_LOGD(TAG, "Sent touch coordinates: %s", coords.c_str());

    // Schedule clearing the state after a delay
    // Use a lambda to capture 'this' correctly
    App.scheduler.set_timeout(this, "clear_touch_sensor", 200, [this]() {
      if (this->touch_coordinates_sensor_ != nullptr) {
        ESP_LOGD(TAG, "Clearing touch coordinates sensor state");
        this->touch_coordinates_sensor_->publish_state("");
      }
    });
  }
}

// Called from Python config
void M5PaperS3DisplayM5GFX::set_touch_sensor(text_sensor::TextSensor *touch_coordinates_sensor) {
  ESP_LOGD(TAG, "Setting touch_coordinates_sensor...");
  this->touch_coordinates_sensor_ = touch_coordinates_sensor;
  // Interval timer is now started in setup() after checking M5.Touch.isEnabled()
  // ESP_LOGD(TAG, "Touch_coordinates_sensor is set");
  // this->set_interval("touch_poll", 100, [this]() { this->update_touch(); }); // Moved to setup
}
// ==============================


void M5PaperS3DisplayM5GFX::dump_config() /* override */ {
    LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper (4-bit Grayscale)", this);
    ESP_LOGCONFIG(TAG, "  Rotation: %d degrees", this->rotation_ * 90);
    if (this->canvas_) {
        ESP_LOGCONFIG(TAG, "  Canvas Size: %d x %d", this->canvas_->width(), this->canvas_->height());
        ESP_LOGCONFIG(TAG, "  Canvas Color Depth: %d bits", this->canvas_->getColorDepth());
    } else {
        ESP_LOGCONFIG(TAG, "  Canvas: Not Initialized");
    }
    ESP_LOGCONFIG(TAG, "  Touch Sensor: %s", YESNO(this->touch_coordinates_sensor_ != nullptr));
}

M5PaperS3DisplayM5GFX::~M5PaperS3DisplayM5GFX() /* override */ {
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
    // Apply rotation to the actual display object if it's already initialized
    if (gfx.width() > 0) { // Check if display is initialized
        gfx.setRotation(this->rotation_);
    }
    // Note: Changing rotation after setup might require canvas recreation
    // if the dimensions change relative to the physical display.
}

// Use canvas dimensions if available, otherwise fallback to gfx_ dimensions
int M5PaperS3DisplayM5GFX::get_width_internal() /* override */ {
    return (this->canvas_) ? this->canvas_->width() : gfx.width();
}

int M5PaperS3DisplayM5GFX::get_height_internal() /* override */ {
    return (this->canvas_) ? this->canvas_->height() : gfx.height();
}

void M5PaperS3DisplayM5GFX::fill(Color color) /* override */ {
    if (this->canvas_ == nullptr) return;
    uint8_t palette_index = get_grayscale_palette_index(color);
    //ESP_LOGV(TAG, "fill() called, Color Brightness: %.2f -> Palette Index: %d", color.get_brightness(), palette_index);
    this->canvas_->fillSprite(palette_index);
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) /* override */ {
    if (this->canvas_ == nullptr) return;
    // Use canvas dimensions for bounds check
    if (x < 0 || x >= this->canvas_->width() || y < 0 || y >= this->canvas_->height()) {
        return;
    }
    uint8_t palette_index = get_grayscale_palette_index(color);
    // ESP_LOGV(TAG, "draw_pixel_at(%d, %d), Color Brightness: %.2f -> Palette Index: %d", x, y, color.get_brightness(), palette_index); // Can be very verbose
    this->canvas_->drawPixel(x, y, palette_index);
}

void M5PaperS3DisplayM5GFX::set_writer(std::function<void(display::Display &)> writer) {
  this->writer_ = writer;
}

// Removed implementation for draw_absolute_pixel_internal

} // namespace m5papers3_display_m5gfx
} // namespace esphome
