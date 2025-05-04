#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/display/display.h"
#include "esphome/components/text_sensor/text_sensor.h"
// No need for display_buffer.h if inheriting display::Display directly

// Includes for M5GFX and M5Unified
#include <M5Unified.h> // For M5.begin() and M5.Display object
#include <M5GFX.h>      // For LGFX types

// Forward declaration for TextSensor
namespace esphome {
namespace text_sensor { class TextSensor; }
}

// Forward declaration for LGFX_Sprite (usually in M5GFX.h, but good practice)
namespace lgfx { class LGFX_Sprite; }


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
    // get_setup_priority is inherited from Component via Display
    float get_setup_priority() const override { return setup_priority::HARDWARE; }
    void update() override; // Override update from Component/Display
    // Destructor does NOT need override unless base class has virtual destructor AND you need specific cleanup.
    // display::Display does not have a virtual destructor, Component does.
    // Remove override keyword.
    ~M5PaperS3DisplayM5GFX(); // Removed override

    // --- Display Methods ---
    // These override methods from display::Display
    void fill(Color color) override;
    int get_width_internal() override;
    int get_height_internal() override;
    void draw_pixel_at(int x, int y, esphome::Color color) override;

    display::DisplayType get_display_type() override {
        // Indicate this display supports grayscale
        return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
    }

    // --- Custom Methods ---
    void set_rotation(int rotation);
    void set_writer(std::function<void(display::Display &)> writer);

    // --- Touch Methods ---
    void set_touch_sensor(text_sensor::TextSensor *sensor);
    // void handle_touch(uint16_t x, uint16_t y); // Seems unused? Keep if needed later.


 protected:
    // --- Internal Helper Methods ---
    void update_touch(); // Internal polling/update logic
    bool get_touch(TouchPoint *point); // Get raw touch data
    void send_coordinates(TouchPoint tp); // Publish touch data

    // --- Member Variables ---
    int rotation_{0}; // Store M5GFX rotation (0, 1, 2, 3)
    lgfx::LGFX_Sprite *canvas_{nullptr}; // Pointer to the sprite buffer (our canvas)
    std::function<void(display::Display &)> writer_{nullptr}; // Lambda for drawing

    // Use a reference to the global M5.Display object, initialized by M5.begin()
    m5gfx::LGFX_Device& gfx_ = M5.Display; // Correct member name gfx_

    // Touch related members
    text_sensor::TextSensor *touch_coordinates_sensor_{nullptr}; // Correct member name touch_coordinates_sensor_

    // Removed unused members from original header:
    // M5GFX gfx; // Removed, use gfx_ reference instead
    // bool touch_detected_{false};
    // uint16_t touch_x_{0}, touch_y_{0};
    // void draw_absolute_pixel_internal(int x, int y, esphome::Color color); // Not needed for display::Display inheritance with own buffer
};

} // namespace m5papers3_display_m5gfx
} // namespace esphome
