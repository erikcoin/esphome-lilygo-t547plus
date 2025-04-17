#include "my_display.h"

namespace my_display22 {

MyEpaperDisplay::MyEpaperDisplay() {}

void MyEpaperDisplay::setup() {
  gfx.begin();
  gfx.setRotation(0);
  gfx.setTextColor(0x0000);  // zwart
  gfx.fillScreen(0xFFFF);    // wit
}

void MyEpaperDisplay::update() {
  // Roep ESPHome's draw routine aan
    this->gfx.fillScreen(TFT_WHITE);
  this->gfx.setTextColor(TFT_BLACK);
  this->gfx.setCursor(10, 10);
  this->gfx.setTextSize(2);
  this->gfx.print("Hello EPD");
  this->gfx.display();  // heel belangrijk!
  this->do_update_();
}

void MyEpaperDisplay::draw_absolute_pixel_internal(int x, int y, esphome::Color color) {
  // Zwart of wit
  uint16_t col = color.is_on() ? 0x0000 : 0xFFFF;
  gfx.drawPixel(x, y, col);
}

void MyEpaperDisplay::fill(esphome::Color color) {
  uint16_t col = color.is_on() ? 0x0000 : 0xFFFF;
  gfx.fillScreen(col);
}

// === Verplichte overrides ===

esphome::display::DisplayType MyEpaperDisplay::get_display_type() {
  return esphome::display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
}

int MyEpaperDisplay::get_width_internal() {
  return 960;
}

int MyEpaperDisplay::get_height_internal() {
  return 540;
}

}  // namespace my_display2
