#include "my_display.h"
#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/components/display/display.h"
//namespace esphome {
namespace my_display22 {

MyEpaperDisplay::MyEpaperDisplay() {}

void MyEpaperDisplay::setup() {
  bool ok = gfx.begin();
  ESP_LOGD("my_display", "gfx.begin() resultaat: %s", ok ? "OK" : "MISLUKT");
  //gfx.begin();
  gfx.setRotation(0);
  gfx.fillScreen(TFT_WHITE);
  gfx.display();
}

void MyEpaperDisplay::update() {
  ESP_LOGD("my_display", "TEST: update wordt aangeroepen");

  gfx.fillScreen(TFT_WHITE);
  gfx.setTextColor(TFT_BLACK);
  gfx.setCursor(20, 20);
  gfx.print("DIT IS EEN TEST OM TEI ZIEN OF IE HET GOED OET");
  gfx.display();
  //gfx.fillScreen(TFT_WHITE);     // wis scherm
  //this->do_update_();            // ESPHome lambda
  //gfx.display();                 // laat zien
}

void MyEpaperDisplay::draw_absolute_pixel_internal(int x, int y, esphome::Color color) {
  ESP_LOGD("my_display", "TEST: draw_absolute_pixel_internal wordt aangeroepen");
  uint16_t col = color.is_on() ? TFT_BLACK : TFT_WHITE;
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
