#include "papers33.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/color.h" // Include for Color definition
#include <cmath> // Include for roundf
#include <algorithm> // Include for std::max (if not using color.gray())

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

// Helper function to convert ESPHome Color to a 4-bit grayscale palette index (0-15)
// Assumes palette index 0 is white and 15 is black.
static inline uint8_t get_grayscale_palette_index(esphome::Color color) {
    // Use color.gray() which returns a grayscale value (0-255)
    // 0 = black, 255 = white
    float gray_value = color.gray(); // Get the grayscale value (0-255)

    // Map gray_value [0, 255] to palette index [15, 0]
    // gray_value 0 (black) should map to index 15
    // gray_value 255 (white) should map to index 0
    float index_float = (255.0f - gray_value) / 255.0f * 15.0f;
    uint8_t index = static_cast<uint8_t>(roundf(index_float));

    // Ensure index is within bounds [0, 15]
    index = std::min((uint8_t)15, std::max((uint8_t)0, index));

    return index; // Returns 0 for white, 15 for black, and intermediates
}


void M5PaperS3DisplayM5GFX::setup() {
    ESP_LOGD(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    auto cfg = M5.config();
    // Consider enabling PSRAM if you run into memory issues with larger sprites/depths
    // cfg.set_psram(true); // Uncomment if needed and PSRAM is available
    M5.begin(cfg);

    ESP_LOGD(TAG, "M5.begin() finished.");

    // *** Change 1: Use epd_quality for better grayscale, though potentially slower ***
    // You could keep epd_fast here and set epd_quality only during update()
    // M5.Display.setEpdMode(epd_mode_t::epd_fast);
    M5.Display.setEpdMode(epd_mode_t::epd_quality); // Set quality mode for grayscale

    while (!M5.Display.isReadable()) {
        ESP_LOGD(TAG, "Waiting for EPD to be ready...");
        delay(1000);
    }

    // Correct member name touch_coordinates_sensor_
    if (this->touch_coordinates_sensor_ != nullptr) {
        ESP_LOGD(TAG, "Publishing test value to touch sensor.");
        // Correct member name touch_coordinates_sensor_
        this->touch_coordinates_sensor_->publish_state("42,84"); // Initial test value
    }

    auto &gfx = M5.Display; // Use reference for convenience
    gfx.setRotation(this->rotation_);
    ESP_LOGD(TAG, "M5GFX Rotation set to: %d", this->rotation_);


    if (this->canvas_ != nullptr) {
      delete this->canvas_;
    }
    this->canvas_ = new lgfx::LGFX_Sprite(&gfx);

    // *** Change 2: Set Color Depth to 4 bits for 16 colors (grayscale levels) ***
    this->canvas_->setColorDepth(4);
    ESP_LOGD(TAG, "Canvas color depth set to 4 bits.");

    // *** Change 3: Set the palette to grayscale ***
    // This automatically creates a 16-level grayscale palette for the 4-bit depth
    // Index 0 will be white (or lightest gray), Index 15 will be black (or darkest gray)
    this->canvas_->setPaletteGrayscale();
    ESP_LOGD(TAG, "Canvas palette set to grayscale.");

    // Alternative: Manually create the palette (if setPaletteGrayscale() causes issues)
    /*
    ESP_LOGD(TAG, "Manually setting grayscale palette...");
    for (int i = 0; i < 16; ++i) {
        // Formula for 16 levels from white (255) to black (0)
        // uint8_t level = 255 - (i * 17); // i=0 => 255 (white), i=15 => 0 (black) - Reversed from helper!
        // Let's match the helper: index 0 = white, index 15 = black
          uint8_t level = (15 - i) * 17; // i=0 => 255 (white), i=15 => 0 (black)
        // color888 creates a 24-bit color M5GFX understands
        this->canvas_->setPaletteColor(i, gfx.color888(level, level, level));
        ESP_LOGD(TAG, "Palette index %d set to gray level %d", i, level);
    }
    */

    bool ok = this->canvas_->createSprite(gfx.width(), gfx.height());
    if (!ok) {
        ESP_LOGE(TAG, "Failed to create canvas sprite! Check memory (try enabling PSRAM?).");
        // Handle error, maybe delete canvas_?
        delete this->canvas_;
        this->canvas_ = nullptr;
    } else {
        ESP_LOGD(TAG, "Canvas created with size: %d x %d", gfx.width(), gfx.height());
        // Clear the canvas initially (using palette index 0 = white)
        this->canvas_->fillSprite(0);
    }

    if (!M5.Touch.isEnabled()) {
        ESP_LOGW(TAG, "Touchscreen not enabled or GT911 not found.");
    } else {
        ESP_LOGI(TAG, "Touchscreen initialized.");
    }

    // Initial display clear might be desired by some users, but often avoided on E-Paper
    // gfx.clearDisplay(TFT_WHITE); // Clear physical screen once if needed
    // gfx.display();
}

void M5PaperS3DisplayM5GFX::update() {
    static bool first_time = true;
    if (first_time) {
        ESP_LOGD(TAG, "Delaying first update for EPD...");
        // The wait in setup() might be sufficient, this delay might be removable
        delay(1000);
        first_time = false;
    }

    // Ensure canvas exists
    if (this->canvas_ == nullptr) {
        ESP_LOGE(TAG, "Canvas not available in update()!");
        return;
    }

    // *** Change 4: Ensure EPD mode is set for quality grayscale rendering ***
    // This is crucial right before pushing the sprite for grayscale
    M5.Display.setEpdMode(epd_mode_t::epd_quality);

    if (this->writer_ != nullptr) {
        ESP_LOGD(TAG, "Clearing canvas sprite (fill with index 0 = white)");
        // Clear the sprite buffer with the palette index for white (usually 0)
        this->canvas_->fillSprite(0); // Index 0 should be white with setPaletteGrayscale

        ESP_LOGD(TAG, "Calling writer lambda...");
        this->writer_(*this); // Let the lambda draw onto the canvas

        ESP_LOGD(TAG, "Pushing sprite to display buffer...");
        // Push the entire canvas sprite to the display controller's buffer
        // Parameters are destination x, y on the physical screen
        this->canvas_->pushSprite(0, 0);

        ESP_LOGD(TAG, "Triggering EPD refresh (display)...");
        // Correct member name gfx_
        this->gfx_.display();
        // Optional: Wait for display update to finish if needed for timing
        // this->gfx_.waitDisplay(); // Correct member name gfx_

        // Optional: Switch back to a faster mode if subsequent operations need it
        // M5.Display.setEpdMode(epd_mode_t::epd_fast);
    } else {
        ESP_LOGD(TAG, "No writer lambda set, skipping drawing.");
        // Optionally, still trigger display if you want the clear to show
        // this->canvas_->pushSprite(0, 0);
        // this->gfx_.display(); // Correct member name gfx_
    }

    // Update touch separately, happens regardless of writer
    update_touch();
}

// ======= Touch related functions remains the same except member name corrections =======
bool M5PaperS3DisplayM5GFX::get_touch(TouchPoint *point) {
    m5::touch_point_t tp[1];
    int touch = M5.Display.getTouchRaw(tp, 1);
    if (touch > 0) {
        // ESP_LOGV(TAG, "Raw touch detected at (%d, %d)", tp[0].x, tp[0].y); // Use Verbose for frequent logs
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

  // ESP_LOGD(TAG, "Raw touch detected at (%d, %d)", point.x, point.y); // Logged in get_touch

  // Correct member name touch_coordinates_sensor_
  if (this->touch_coordinates_sensor_ == nullptr) {
    // ESP_LOGW(TAG, "Touch coordinates sensor not initialized!"); // Only warn once maybe?
    return;
  }

  this->send_coordinates(point);
}

void M5PaperS3DisplayM5GFX::send_coordinates(TouchPoint tp) {
  // Correct member name touch_coordinates_sensor_
  if (this->touch_coordinates_sensor_ != nullptr) {
    std::string coords = std::to_string(tp.x) + "," + std::to_string(tp.y);
    // Correct member name touch_coordinates_sensor_
    this->touch_coordinates_sensor_->publish_state(coords);
    // ESP_LOGV(TAG, "Sending coordinates: %s", coords.c_str()); // Use Verbose

    // Clear state after a short delay
    // Use the correct method for scheduling from Component base class (inherited via Display)
    this->set_timeout("clear_touch_sensor", 200, [this]() {
      // Correct member name touch_coordinates_sensor_
      if (this->touch_coordinates_sensor_ != nullptr)
        // Correct member name touch_coordinates_sensor_
        this->touch_coordinates_sensor_->publish_state("");
    });
  }
}

void M5PaperS3DisplayM5GFX::set_touch_sensor(text_sensor::TextSensor *touch_coordinates_sensor) {
  ESP_LOGD(TAG, "Setting touch_coordinates_sensor...");
  // Correct member name touch_coordinates_sensor_
  this->touch_coordinates_sensor_ = touch_coordinates_sensor;
  ESP_LOGD(TAG, "Touch_coordinates_sensor is set");

  // Setup interval polling for touch updates
  // This calls the set_interval from the Component base class (no longer ambiguous)
  this->set_interval("touch_poll", 100, [this]() { this->update_touch(); }); // Give interval a name
}
// ========================================================


void M5PaperS3DisplayM5GFX::dump_config() {
    LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper (4-bit Grayscale)", this);
    // Display rotation in degrees
    int display_rotation_deg = this->rotation_ * 90;
    ESP_LOGCONFIG(TAG, "  Rotation: %d degrees", display_rotation_deg);
    // Log canvas info if available
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
        default: m5gfx_rotation = 0; break; // Case 0
    }
    this->rotation_ = m5gfx_rotation;
    // The actual M5GFX rotation will be applied in setup() or maybe update()
    // Applying it here might be too early or require canvas recreation.
    // If M5.Display is already initialized, you *could* set it, but doing it in setup is safer.
    // If you change rotation dynamically after setup, you'd need to handle canvas recreation.
}

int M5PaperS3DisplayM5GFX::get_width_internal() {
    // Return canvas width if exists, otherwise display width
    // Note: M5GFX width/height changes based on rotation.
    return (this->canvas_) ? this->canvas_->width() : M5.Display.width();
}

int M5PaperS3DisplayM5GFX::get_height_internal() {
    // Return canvas height if exists, otherwise display height
    // Note: M5GFX width/height changes based on rotation.
    return (this->canvas_) ? this->canvas_->height() : M5.Display.height();
}

// *** Change 5: Use grayscale mapping ***
void M5PaperS3DisplayM5GFX::fill(Color color) {
    if (this->canvas_ == nullptr) return;
    uint8_t palette_index = get_grayscale_palette_index(color);
    this->canvas_->fillSprite(palette_index);
}

// *** Change 6: Use grayscale mapping ***
void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
    if (this->canvas_ == nullptr) return;
    // Check bounds
    if (x < 0 || x >= this->get_width_internal() || y < 0 || y >= this->get_height_internal()) {
        return;
    }
    uint8_t palette_index = get_grayscale_palette_index(color);
    this->canvas_->drawPixel(x, y, palette_index);
}

// The writer function is set externally via the lambda in YAML
void M5PaperS3DisplayM5GFX::set_writer(std::function<void(display::Display &)> writer) {
  this->writer_ = writer;
}

} // namespace m5papers3_display_m5gfx
} // namespace esphome
