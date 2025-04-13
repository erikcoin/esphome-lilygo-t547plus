#include "erik.h"
#include "esphome/core/log.h"

namespace esphome {
namespace erik {

void ErikDisplay::setup() {
  M5.begin();
  M5.Display.setRotation(3);
  draw_screen_();
}

void ErikDisplay::loop() {
  M5.update();

  auto touch = M5.Touch.getDetail();
  bool is_touched = touch.wasPressed();

  if (is_touched && !last_touch_state_ && is_touch_inside_button_()) {
    if (this->touch_callback_) {
      this->touch_callback_();
    }
  }

  last_touch_state_ = is_touched;
}

void ErikDisplay::show_state(bool on) {
  light_on_ = on;
  draw_screen_();
}

void ErikDisplay::draw_screen_() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(30, 30);
  M5.Display.print("Bibliotheeklamp");

  // Draw toggle button
  int x = 100, y = 150, w = 200, h = 60;
  uint32_t color = light_on_ ? TFT_GREEN : TFT_RED;

  M5.Display.fillRoundRect(x, y, w, h, 10, color);
  M5.Display.setTextColor(TFT_BLACK);
  M5.Display.setCursor(x + 40, y + 20);
  M5.Display.print(light_on_ ? "Turn Off" : "Turn On");
}

bool ErikDisplay::is_touch_inside_button_() {
  m5::touch_detail_t t = M5.Touch.getDetail();
  int x = t.x;
  int y = t.y;

  // Button bounds
  int bx = 100, by = 150, bw = 200, bh = 60;

  return (x >= bx && x <= (bx + bw) && y >= by && y <= (by + bh));
}

}  // namespace erik
}  // namespace esphome
