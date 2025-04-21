#pragma once

#include "esphome/components/display/display_buffer.h"
#include "esphome/core/component.h"
#include <M5GFX.h>

namespace esphome {
namespace my_display {

class MyEpaperDisplay : public display::DisplayBuffer {
 public:
  M5GFX gfx;
  M5Canvas canvas;
  void setup() override;
  void update() override;
  void dump_config() override {}

  int get_width() override { return 960; }
  int get_height() override { return 540; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override {
    // voorlopig geen individuele pixels tekenen
  }

  display::DisplayType get_display_type() override {
    return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
  }

  int get_width_internal() override { return get_width(); }
  int get_height_internal() override { return get_height(); }
};

}  // namespace my_display
}  // namespace esphome
