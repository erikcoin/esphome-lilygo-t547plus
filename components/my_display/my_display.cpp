#include "my_display.h"
#include "esphome/core/log.h"
#include "esphome/core/component.h"
#include "esphome/components/display/display.h"
namespace esphome {
namespace my_display22 {

MyEpaperDisplay::MyEpaperDisplay() {}

void MyEpaperDisplay::setup() {
  ESP_LOGD("my_display", "setup wordt upgevoerd");
  gfx.begin();
  gfx.setRotation(0);
  // Init canvas met juiste grootte en 1-bit kleur
  canvas.setColorDepth(1);  // 1-bit zwart/wit. Of 4 voor grayscale
  canvas.setFont(&fonts::Font0);  // optioneel, fallback
  canvas.createSprite(get_width_internal(), get_height_internal());
  canvas.setTextColor(TFT_BLACK, TFT_WHITE);
  canvas.fillScreen(TFT_WHITE);
  //gfx.clear();              // wist interne framebuffer (optioneel)
  //gfx.fillScreen(TFT_BLACK); // teken volledig wit
  gfx.display();
}

void MyEpaperDisplay::update() {
ESP_LOGD("my_display", "Update wordt uitgevoerd");
   // Canvas wit maken voor nieuwe frame
  canvas.fillScreen(TFT_WHITE);  // 1 = wit in 1-bit
  //Dit werkt, laat de tekst op het display zien
  // this->gfx.fillScreen(TFT_WHITE);
  // this->gfx.setTextColor(TFT_BLACK);
  //this->gfx.setCursor(10, 10);
  //this->gfx.setTextSize(2);
  //this->gfx.print("Hello EPD");
  this->do_update_(); // roept draw_absolute_pixel_internal(x, y, color) aan
    // Push het canvas naar het scherm
  gfx.startWrite();  // optioneel
  canvas.pushSprite(0, 0);       // Canvas pushen naar scherm
  //gfx.pushImage(0, 0, canvas.width(), canvas.height(), (uint16_t *) canvas.getBuffer());
  gfx.endWrite();
  
  this->gfx.display();  // heel belangrijk!
}

void MyEpaperDisplay::draw_absolute_pixel_internal(int x, int y, esphome::Color color) {
  // Zwart of wit

  //extra logging
  ESP_LOGD("my_display", "Pixel at (%d, %d): %s", x, y, color.is_on() ? "on" : "off");
  ESP_LOGD("my_display", "draw_absolute_pixel wordt uitgevoerd");
  //einde exta logging
  //uint16_t col = color.is_on() ? 0x0000 : 0xFFFF;
  //gfx.drawPixel(x, y, col);
  bool on = color.is_on(); // zwart = true
  canvas.drawPixel(x, y, on ? 0 : 1); // 0 = zwart, 1 = wit (voor 1-bit canvas)
}

void MyEpaperDisplay::fill(esphome::Color color) {
  ESP_LOGD("my_display", "prodedure fill aangeroepen");
  canvas.fillScreen(color.is_on() ? 0 : 1); // 0 = zwart, 1 = wit
  // uint16_t col = color.is_on() ? 0x0000 : 0xFFFF;
 // gfx.fillScreen(col);
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
} //namespace esphome
