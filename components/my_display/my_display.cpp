#include "my_display.h"
#include <epdiy.h>
#include <M5GFX.h>
//#include <epd_driver.h>

MyEpaperDisplay::MyEpaperDisplay() : DisplayBuffer(960, 540) {}

void MyEpaperDisplay::setup() {
  epd_init();
  gfx.init();
  gfx.setRotation(1);
  gfx.setTextColor(EPD_BLACK);
  gfx.setTextSize(1);
  gfx.fillScreen(EPD_WHITE);
  gfx.display();
}

void MyEpaperDisplay::update() {
  this->do_draw_();
  gfx.display();
}

void MyEpaperDisplay::draw_absolute_pixel_internal(int x, int y, esphome::Color color) {
  gfx.drawPixel(x, y, color.is_on() ? EPD_BLACK : EPD_WHITE);
}

void MyEpaperDisplay::fill(esphome::Color color) {
  gfx.fillScreen(color.is_on() ? EPD_BLACK : EPD_WHITE);
}
