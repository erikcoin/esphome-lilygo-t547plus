#include "erik.h"

namespace esphome {
namespace erik {

void Erik::setup() {
  display_.begin();
  display_.setRotation(1);  // afhankelijk van je schermoriëntatie
  display_.fillScreen(TFT_WHITE);
}

void Erik::update() {
  this->do_update_();  // zorgt ervoor dat ESPHome intern het buffer opnieuw tekent
  display_.display();  // in sommige m5gfx drivers niet nodig, afhankelijk van implementatie
}

void Erik::draw_absolute_pixel_internal(int x, int y, Color color) {
  uint16_t col = color.is_on() ? TFT_WHITE : TFT_BLACK;
  display_.drawPixel(x, y, col);
}

}  // namespace erik
}  // namespace esphome
