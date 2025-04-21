#include "my_display.h"

namespace esphome {
namespace my_display {

void MyEpaperDisplay::setup() {
  gfx.begin();
  gfx.setEpdMode(epd_mode_t::epd_fastest);
  gfx.setRotation(0); // of 1/2/3 afhankelijk van hoe je scherm gemonteerd is
  gfx.fillScreen(TFT_WHITE);
  gfx.display();
  ESP_LOGI("my_display", "Scherm wit gemaakt bij opstart.");
}

void MyEpaperDisplay::update() {
  // geen rendering via lambda nodig hier
}

}  // namespace my_display
}  // namespace esphome
