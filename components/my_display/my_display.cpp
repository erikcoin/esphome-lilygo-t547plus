#include "my_display.h"

MyEpaperDisplay::MyEpaperDisplay() {}

void MyEpaperDisplay::setup() {
  this->set_width(960);
  this->set_height(540);
  this->set_writer([](esphome::display::Display *d) {});

  gfx.begin();
  gfx.setRotation(1);
  gfx.setTextColor(BLACK);
  gfx.setTextSize(1);
  gfx.fillScreen(WHITE);
  gfx.display();
}

void MyEpaperDisplay::update() {
  // Niets nodig als je lambda in YAML gebruikt
  gfx.display();
}

void MyEpaperDisplay::draw_absolute_pixel_internal(int x, int y, esphome::Color color) {
  gfx.drawPixel(x, y, color.is_on() ? BLACK : WHITE);
}

void MyEpaperDisplay::fill(esphome::Color color) {
  gfx.fillScreen(color.is_on() ? BLACK : WHITE);
}
