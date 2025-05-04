#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h" // May not be strictly needed here
#include "esphome/core/log.h"
#include "esphome/components/display/display.h"
#include "esphome/components/text_sensor/text_sensor.h"
// #include "esphome/components/display/display_buffer.h" // Not needed if inheriting Display

// Includes for M5GFX and M5Unified
#include <M5Unified.h> // For M5.begin() and M5.Display object
#include <M5GFX.h>     // For LGFX types

// Forward declaration for LGFX_Sprite if needed, M5GFX.h should cover it
// namespace lgfx { class LGFX_Sprite; } // Usually provided by M5GFX.h include

// Forward declaration for TextSensor
namespace esphome {
namespace text_sensor { class TextSensor; }
}


namespace esphome {
namespace m5papers3_display_m5gfx {

// Custom TouchPoint struct (as provided)
struct TouchPoint {
  uint16_t x;
  uint16_t y;
};

class M5PaperS3DisplayM5GFX : public display::Display { //, public Component { // Also inherit Component
 public:
  // --- Component Lifecycle ---
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void update() override; // Add override
  ~M5PaperS3DisplayM5GFX(); //override; // Add override

  // --- Display Methods ---
  // These override methods from display::Display
  void fill(Color color) override; // Add override
  int get_width_internal() override;
  int get_height_internal() override;
  void draw_pixel_at(int x, int y, esphome::Color color) override; // Add override

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
  m5gfx::LGFX_Device& gfx_ = M5.Display;

  // Touch related members
  text_sensor::TextSensor *touch_coordinates_sensor_{nullptr}; // Sensor entity pointer
  // TouchPoint touch_point_; // Could be local var in update_touch if not needed persistently

 // Removed unused members from original header:
 // M5GFX gfx; // Removed, use gfx_ reference instead
 // bool touch_detected_{false};
 // uint16_t touch_x_{0}, touch_y_{0};
 // void draw_absolute_pixel_internal(int x, int y, esphome::Color color); // Not needed for display::Display inheritance with own buffer
};

} // namespace m5papers3_display_m5gfx
} // namespace esphome
