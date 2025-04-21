#include "my_display.h"

namespace esphome {
namespace my_display {

void MyEpaperDisplay::setup() {
  gfx.begin();
  gfx.setEpdMode(epd_mode_t::epd_quality);  // of epd_fast
  gfx.fillScreen(TFT_WHITE);
  gfx.display();  // push naar scherm
}

void MyEpaperDisplay::update() {
  // Niks doen, enkel voor compatibiliteit
}

}  // namespace my_display
}  // namespace esphome
