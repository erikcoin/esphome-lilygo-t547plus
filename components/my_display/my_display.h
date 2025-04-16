#include "esphome.h"
#include <M5GFX.h>
#include <epd_driver.h>

class MyEpaperDisplay : public esphome::display::DisplayBuffer {
 public:
  MyEpaperDisplay() : DisplayBuffer(960, 540) {}

  void setup() override {
    epd_init();
    gfx.init();
    gfx.setRotation(1);
    gfx.setTextColor(EPD_BLACK);
    gfx.setTextSize(1);
    gfx.fillScreen(EPD_WHITE);
    gfx.display();
  }

  void update() override {
    this->do_draw_();  // ESPHome lambda draw-calls
    gfx.display();     // Refresh e-paper
  }

  void draw_absolute_pixel_internal(int x, int y, esphome::Color color) override {
    gfx.drawPixel(x, y, color.is_on() ? EPD_BLACK : EPD_WHITE);
  }

  void fill(esphome::Color color) override {
    gfx.fillScreen(color.is_on() ? EPD_BLACK : EPD_WHITE);
  }

 protected:
  M5GFX gfx;
};
