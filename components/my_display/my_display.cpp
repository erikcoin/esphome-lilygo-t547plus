#include "my_display.h"
#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/components/display/display.h"
namespace esphome {
namespace my_display22 {

MyEpaperDisplay::MyEpaperDisplay() {}

void MyEpaperDisplay::setup() {
  ESP_LOGD("my_display", "setup wordt uitgevoerd");

  gfx.begin();
  gfx.setRotation(0);
  gfx.setEpdMode(epd_mode_t::epd_quality);  // optioneel, maar helpt tegen ghosting

  // Init canvas
  canvas.setColorDepth(1);  // 1-bit voor e-paper
  canvas.setFont(&fonts::Font0);  // optioneel
  canvas.setTextColor(TFT_BLACK, TFT_WHITE);
  canvas.createSprite(get_width_internal(), get_height_internal());
  canvas.fillScreen(TFT_WHITE);  // Wit canvas maken
  gfx.fillScreen(TFT_WHITE);
  gfx.display();  // Ververs scherm met huidige framebuffer
}

void MyEpaperDisplay::update() {
  //this->gfx.fillScreen(TFT_WHITE);
  //this->do_update_();      // Laat ESPHome tekenen via lambda
  //this->gfx.display();     // Toon het resultaat
  canvas.fillScreen(TFT_WHITE);  // wis canvas
  this->do_update_();            // ESPHome lambda tekent pixels

  gfx.startWrite();
  gfx.pushImage(0, 0, canvas.width(), canvas.height(), (uint16_t *) canvas.getBuffer());
  gfx.endWrite();

  gfx.display();    // stuur buffer naar scherm
  //gfx.hibernate();  // voorkom ghosting
  
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

display::DisplayType MyEpaperDisplay::get_display_type() {
  return display::DisplayType::DISPLAY_TYPE_BINARY;
}

int MyEpaperDisplay::get_width_internal() {
  return 960;
}

int MyEpaperDisplay::get_height_internal() {
  return 540;
}

}  // namespace my_display22
} //namespace esphome
