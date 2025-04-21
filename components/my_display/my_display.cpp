#include "my_display.h"

namespace esphome {
namespace my_display {

void MyEpaperDisplay::setup() {
  gfx.begin();
  gfx.fillScreen(TFT_BLACK);
  x = gfx.width() / 2;
  y = gfx.height() / 2;
  //gfx.setEpdMode(epd_mode_t::epd_fastest);
  canvas.createSprite(50, 50);
  canvas.fillSprite(TFT_WHITE);
  canvas.fillRect(10, 10, 20, 20, TFT_BLACK);
  canvas.println("M5Canvas");

  // Only the following process is actually drawn on the panel.
  gfx.startWrite(); 
  gfx.println("Display");
  gfx.pushSprite(x, y);
  display.endWrite();
  ESP_LOGI("my_display", "Scherm wit gemaakt bij opstart.");
}

void MyEpaperDisplay::update() {
  // geen rendering via lambda nodig hier
}

}  // namespace my_display
}  // namespace esphome
