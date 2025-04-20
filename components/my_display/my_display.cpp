#include "my_display.h"
#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/components/display/display.h"
//namespace esphome {
namespace my_display22 {

MyEpaperDisplay::MyEpaperDisplay() {}

void MyEpaperDisplay::setup() {
  //ESP_LOGD("my_display", "setup wordt upgevoerd");
  gfx.begin();
  gfx.setRotation(0);
  // Forceer volledige refresh
  gfx.clear();              // wist interne framebuffer (optioneel)
  //gfx.fillScreen(TFT_BLACK); // teken volledig wit
  //gfx.display();
  gfx.fillScreen(0xFFFF); // teken volledig wit
  gfx.display();
}

void MyEpaperDisplay::update() {
  // Roep ESPHome's draw routine aan
 // this->gfx.fillScreen(TFT_WHITE);
  this->gfx.setTextColor(TFT_BLACK);
//  this->do_update_();      // Laat ESPHome tekenen wat jij in YAML schrijft
//  this->gfx.display();     // Pas daarna tonen
ESP_LOGD("my_display", "Update wordt uitgevoerd");
  //Dit werkt, laat de tekst op het display zien
  this->gfx.fillScreen(TFT_WHITE);
  this->gfx.setTextColor(TFT_BLACK);
  this->gfx.setCursor(10, 10);
  this->gfx.setTextSize(2);
  this->gfx.print("Hello EPD");
  this->do_update_();
  this->gfx.display();  // heel belangrijk!
}

void MyEpaperDisplay::draw_absolute_pixel_internal(int x, int y, esphome::Color color) {
  // Zwart of wit

  //extra logging
  ESP_LOGD("my_display", "Pixel at (%d, %d): %s", x, y, color.is_on() ? "on" : "off");
  ESP_LOGD("my_display", "draw_absolute_pixel wordt uitgevoerd");
  //einde exta logging
  uint16_t col = color.is_on() ? 0x0000 : 0xFFFF;
  gfx.drawPixel(x, y, col);
}

void MyEpaperDisplay::fill(esphome::Color color) {
  ESP_LOGD("my_display", "prodedure fill aangeroepen");
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

}  // namespace my_display22
//} //namespace esphome
