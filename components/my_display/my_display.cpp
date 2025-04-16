#include "my_display.h"

namespace my_display22 {  // zelfde als in .h

MyEpaperDisplay::MyEpaperDisplay() {}

void MyEpaperDisplay::setup() {
  this->gfx.begin();
  this->gfx.setEpdMode(epd_mode_t::epd_quality);
  this->gfx.setRotation(3);
  this->gfx.setTextColor(TFT_WHITE);
  this->gfx.fillScreen(TFT_WHITE);
  this->gfx.setCursor(10, 10);
  this->gfx.print("Init display");

  this->gfx.display();  // forceer draw
}

void MyEpaperDisplay::update() {
  this->gfx.setCursor(10, 30);
  this->gfx.print("Update");
  this->gfx.display();
}

void MyEpaperDisplay::draw_absolute_pixel_internal(int x, int y, esphome::Color color) {
  this->gfx.drawPixel(x, y, color.is_on() ? TFT_BLACK : TFT_WHITE);
}

void MyEpaperDisplay::fill(esphome::Color color) {
  this->gfx.fillScreen(color.is_on() ? TFT_BLACK : TFT_WHITE);
}

int MyEpaperDisplay::get_width() {
  return 960;
}

int MyEpaperDisplay::get_height() {
  return 540;
}
esphome::display::DisplayType MyEpaperDisplay::get_display_type() {
  return esphome::display::DisplayType::DISPLAY_TYPE_MONOCHROME;  // of MONOCHROME
}

int MyEpaperDisplay::get_width_internal() {
  return 960;
}

int MyEpaperDisplay::get_height_internal() {
  return 540;
}

}  // namespace my_display2
