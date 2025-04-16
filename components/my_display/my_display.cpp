#include "my_display.h"

MyEpaperDisplay::MyEpaperDisplay() {}

void MyEpaperDisplay::setup() {
  gfx.begin();
  gfx.setRotation(1);
  gfx.setTextColor(BLACK);
  gfx.setTextSize(1);
  gfx.fillScreen(WHITE);
  gfx.display();

  this->set_writer([](esphome::display::Display &d) {
    // eventueel: d.print("Test");
  });
}

void MyEpaperDisplay::update() {
  gfx.display(); // Update M5GFX scherm
}

void MyEpaperDisplay::draw_absolute_pixel_internal(int x, int y, esphome::Color color) {
  gfx.drawPixel(x, y, color.is_on() ? BLACK : WHITE);
}

void MyEpaperDisplay::fill(esphome::Color color) {
  gfx.fillScreen(color.is_on() ? BLACK : WHITE);
}

int MyEpaperDisplay::get_width() {
  return 960;
}

int MyEpaperDisplay::get_height() {
  return 540;
}
