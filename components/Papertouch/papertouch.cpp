#include "papertouch.h"
#include "esphome/core/log.h"

namespace esphome {
namespace papertouch {

static const char *const TAG = "papertouch";

void papertouch::setup() {
  ESP_LOGI(TAG, "Papertouch setup");

  if (!M5.Touch.isEnabled()) {
    ESP_LOGI(TAG, "Enabling M5.Touch");
    M5.Touch.begin();
  }
}

void papertouch::update_touches() {
  if (M5.Touch.isEnabled() && M5.Touch.isPressed()) {
    auto point = M5.Touch.getTouchPoint();
    touchscreen::TouchPoint touch_point;
    touch_point.x = point.x;
    touch_point.y = point.y;
    touch_point.id = 0;  // Je kan meer doen als je multitouch wilt
    this->touch_points_.push_back(touch_point);

    ESP_LOGD(TAG, "Touch at x=%d, y=%d", point.x, point.y);
  }
}

}  // namespace papertouch
}  // namespace esphome
