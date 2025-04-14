#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include <epdiy.h>
#include <M5GFX.h>

namespace esphome {
namespace my_display {  // 👈 MUST MATCH display.py namespace

class MyDisplay : public esphome::display::DisplayBuffer, public esphome::Component {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  void draw_absolute_pixel_internal(int x, int y, esphome::Color color) override;
  int get_width_internal() override { return 960; }
  int get_height_internal() override { return 540; }

 private:
  M5GFX display_;
};

}  // namespace my_display
}  // namespace esphome
