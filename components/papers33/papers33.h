#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/display/display.h"
#include "esphome/components/text_sensor/text_sensor.h"
// No need for display_buffer.h if inheriting display::Display directly

// Includes for M5GFX and M5Unified
// These include headers that define LGFX_Sprite, possibly within a namespace like lgfx::v1
#include <M5Unified.h> // For M5.begin() and M5.Display object
#include <M5GFX.h>      // For LGFX types

// Remove the ambiguous forward declaration
// namespace lgfx { class LGFX_Sprite; } // REMOVED

// Forward declaration for TextSensor
namespace esphome {
namespace text_sensor { class TextSensor; }
}

// Explicitly refer to the v1 namespace LGFX_Sprite
namespace lgfx { namespace v1 { class LGFX_Sprite; } } // Add explicit v1 forward declaration if needed, or just use the full name


namespace esphome {
namespace m5papers3_display_m5gfx {

// Custom TouchPoint struct (as provided)
struct TouchPoint {
    uint16_t x;
    uint16_t y;
};

// Inherit ONLY from display::Display. display::Display already inherits from Component.
class M5PaperS3DisplayM5GFX : public display::Display {
 public:
    // --- Component Lifecycle ---
    void setup() override;
    void dump_config() override;
    float get_setup_priority() const override { return setup_priority::HARDWARE; }
    void update() override;
    ~M5PaperS3DisplayM5GFX();

    // --- Display Methods ---
    void fill(Color color) override;
    int get_width_internal() override;
    int get_height_internal() override;
    void draw_pixel_at(int x, int y, esphome::Color color) override;

    display::DisplayType get_display_type() override {
        return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
    }

    // --- Custom Methods ---
    void set_rotation(int rotation);
    void set_writer(std::function<void(display::Display &)> writer);

    // --- Touch Methods ---
    void set_touch_sensor(text_sensor::TextSensor *sensor);

 protected:
    // --- Internal Helper Methods ---
    void update_touch();
    bool get_touch(TouchPoint *point);
    void send_coordinates(TouchPoint tp);

    // --- Member Variables ---
    int rotation_{0};
    // Use the fully qualified name for LGFX_Sprite
    lgfx::v1::LGFX_Sprite *canvas_{nullptr}; // Corrected type
    std::function<void(display::Display &)> writer_{nullptr};

    m5gfx::LGFX_Device& gfx_ = M5.Display;

    text_sensor::TextSensor *touch_coordinates_sensor_{nullptr};
};

} // namespace m5papers3_display_m5gfx
} // namespace esphome
