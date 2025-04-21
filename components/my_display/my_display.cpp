#include "my_display.h"

namespace esphome {
namespace my_display {

void MyEpaperDisplay::setup() {
  gfx.begin();
  gfx.setEpdMode(epd_mode_t::epd_quality);
  gfx.fillScreen(TFT_WHITE);
  gfx.display();
}

void MyEpaperDisplay::update() {
  // geen rendering via lambda nodig hier
}

}  // namespace my_display
}  // namespace esphome
