#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"

#include <M5GFX.h>

namespace esphome {
namespace my_display {

class MyDisplay : public display::DisplayBuffer, public Component {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  int get_width_internal() override { return 960; }  // Adjust to your e-paper size
  int get_height_internal() override { return 540; }

 private:
  M5GFX display_;
};

}  // namespace my_display
}  // namespace esphome
