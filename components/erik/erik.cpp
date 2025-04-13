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

  bool is_touched = M5.Touch.isTouched();
  if (is_touched && !last_touch_state_) {
    if (this->touch_callback_) {
      this->touch_callback_();
    }
  }
  last_touch_state_ = is_touched;
}

void ErikDisplay::update_display() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(40, 50);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(2);
  M5.Display.print("Bibliotheeklamp");

  draw_button_(true);  // hardcoded ON
}

void ErikDisplay::draw_button_(bool state) {
  int x = 100, y = 150, w = 200, h = 60;
  uint32_t color = state ? TFT_GREEN : TFT_RED;

  M5.Display.fillRoundRect(x, y, w, h, 10, color);
  M5.Display.setTextColor(TFT_BLACK);
  M5.Display.setCursor(x + 30, y + 20);
  M5.Display.setTextSize(2);
  M5.Display.print(state ? "Turn Off" : "Turn On");
}

}  // namespace erik
}  // namespace esphome
