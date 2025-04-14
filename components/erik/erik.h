#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/display/display_color_utils.h"
<include <epdiy.h>
#include <M5GFX.h>

namespace esphome {
namespace erik {

class Erik : public esphome::Component, public esphome::display::DisplayBuffer {
 public:
  void setup() override;
  void update() override;
  void draw_absolute_pixel_internal(int x, int y, esphome::Color color) override;

  int get_width() override { return 960; }
  int get_height() override { return 540; }

 protected:
  M5GFX display_;
};

}  // namespace erik
}  // namespace esphome
