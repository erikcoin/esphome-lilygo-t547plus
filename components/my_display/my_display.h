
#pragma once
#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include <M5GFX.h>

namespace esphome {
namespace my_display {

class MyDisplay : public PollingComponent, public display::DisplayBuffer {
 public:
  void setup() override;
  void update() override;
  void dump_config() override {}

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }
  void draw_absolute_pixel_internal(int x, int y, int color) override;

 protected:
  M5GFX gfx_;
  M5Canvas canvas_;
};

}  // namespace my_display
}  // namespace esphome
