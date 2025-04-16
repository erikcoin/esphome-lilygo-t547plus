#pragma once
#include "esphome.h"
#include <epdiy.h>
#include <M5GFX.h>

class MyEpaperDisplay : public esphome::display::DisplayBuffer {
 public:
  MyEpaperDisplay();
  void setup() override;
  void update() override;
  void draw_absolute_pixel_internal(int x, int y, esphome::Color color) override;
  void fill(esphome::Color color) override;

 protected:
  M5GFX gfx;
};
