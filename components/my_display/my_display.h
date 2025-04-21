#pragma once

#include "esphome.h"
#include <M5GFX.h>

namespace esphome {
namespace my_display {

class MyEpaperDisplay : public PollingComponent, public display::DisplayBuffer {
 public:
  M5GFX gfx;

  void setup() override;
  void update() override;
  void dump_config() override {}

  int get_width() override { return 960; }  // of juiste resolutie
  int get_height() override { return 540; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override {}
};

}  // namespace my_display
}  // namespace esphome
