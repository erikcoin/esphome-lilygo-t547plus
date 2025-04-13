#include "erik.h"
#include "esphome/core/log.h"

namespace esphome {
namespace erik {

void ErikDisplay::setup() {
  M5.begin();
  M5.Display.setRotation(3);
}

void ErikDisplay::update() {
  M5.update();

  auto touch = M5.Touch.getDetail();
  bool is_touched = touch.wasPressed();

  if (is_touched && !last_touch_state_) {
    if (this->touch_callback_) {
      this->touch_callback_();
    }
  }

  last_touch_state_ = is_touched;
}

void ErikDisplay::draw(display::DisplayBuffer &it) {
  it.fill(COLOR_ON);  // white background
  it.printf(10, 10, id(font), COLOR_OFF, TextAlign::TOP_LEFT, "Bibliotheeklamp");

  // Draw button
  it.rectangle(50, 100, 160, 50);
  it.printf(130, 125, id(font), COLOR_OFF, TextAlign::CENTER, "Toggle");
}

}  // namespace erik
}  // namespace esphome
