#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/display/display.h"
#include "esphome/components/display/display_buffer.h"

// Includes for M5GFX and M5Unified
#include <M5Unified.h> // For M5.begin() and M5.Display object
#include <M5GFX.h>     // For LGFX and M5GFX types

namespace lgfx { using LGFX_Sprite = ::LGFX_Sprite; } // Bring LGFX_Sprite to the lgfx namespace if not already

namespace esphome {
namespace m5papers3_display_m5gfx {
struct TouchPoint {
  uint16_t x;
  uint16_t y;
};
class M5PaperS3DisplayM5GFX : public display::Display {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }
  void update() override;
  ~M5PaperS3DisplayM5GFX();
  void fill(Color color) override;
  int get_width_internal() override;
  int get_height_internal() override;
  display::DisplayType get_display_type() override {
    return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
  }

  void set_rotation(int rotation);
  void set_writer(std::function<void(display::Display &)> writer);
  void draw_pixel_at(int x, int y, esphome::Color color) override;

  // New methods for touch
  void handle_touch(uint16_t x, uint16_t y);
  void update_touch();
  //bool get_touch(esphome::display::TouchPoint* point);
  bool get_touch(display::TouchPoint *point) override;
 protected:
  void draw_absolute_pixel_internal(int x, int y, esphome::Color color);
  M5GFX gfx;
  lgfx::LGFX_Sprite *canvas_{nullptr};

 private:
  int rotation_{0};
  std::function<void(display::Display &)> writer_{nullptr};
  bool touch_detected_{false};  // Track if a touch is detected
  uint16_t touch_x_{0}, touch_y_{0};  // Touch coordinates
};

} // namespace m5papers3_display_m5gfx
} // namespace esphome
