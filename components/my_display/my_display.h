pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include <M5GFX.h>

namespace esphome {
namespace my_display22 {

class MyEpaperDisplay : public display::DisplayBuffer {
 public:
  MyEpaperDisplay();

  void setup() override;
  void update() override;
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  esphome::display::DisplayType get_display_type() override;
  int get_width_internal() override;
  int get_height_internal() override;

 protected:
  M5GFX gfx_;
};

}  // namespace my_display22
}  // namespace esphome
