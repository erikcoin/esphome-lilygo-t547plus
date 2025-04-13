#include "erik.h"

namespace esphome {
namespace erik {

void ErikComponent::setup() {
  display_.begin();
  display_.setRotation(0);
  display_.fillScreen(TFT_WHITE);
  display_.display();
}

void ErikComponent::update() {
  display_.fillScreen(TFT_WHITE);
  display_.setTextColor(TFT_BLACK);
  display_.setTextSize(size_);
  display_.drawString(text_.c_str(), x_, y_);
  display_.display();  // Push to EPD
}

}  // namespace erik
}  // namespace esphome
