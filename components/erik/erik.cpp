#include "erik.h"

namespace esphome {
namespace erik {

void Erik::setup() {
  display_.begin();
  display_.setRotation(1);
  display_.fillScreen(TFT_WHITE);
}

void Erik::update() {
  this->do_update_();  // ESPHome buffer tekenen
}

void Erik::draw_absolute_pixel_internal(int x, int y, Color color) {
  uint16_t col = color.is_on() ? TFT_BLACK : TFT_WHITE;
  display_.drawPixel(x, y, col);
}

}  // namespace erik
}  // namespace esphome
