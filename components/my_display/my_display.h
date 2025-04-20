#pragma once
#include "esphome.h"
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/version.h"
#include "esphome/components/display/display_buffer.h"

//#include "epd_driver.h"
#include <epdiy.h>
#include <M5GFX.h>

//namespace esphome {
namespace my_display22 {
//deze werkt:
class MyEpaperDisplay : public esphome::display::DisplayBuffer {
//class MyEpaperDisplay : public display::DisplayBuffer {
public:
  MyEpaperDisplay();

  void setup() override;
  void update() override;
  void draw_absolute_pixel_internal(int x, int y, esphome::Color color) override;
  void fill(esphome::Color color) override;

  // Verplichte implementaties voor pure virtuals
  esphome::display::DisplayType get_display_type() override;
  int get_width_internal() override;
  int get_height_internal() override;

 protected:
  M5GFX gfx;
  //M5Canvas canvas;  // ← de nieuwe framebuffer
};

}  // namespace my_display22
//} //namespace esphome
