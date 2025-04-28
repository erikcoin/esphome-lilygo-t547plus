#include "papertouch.h"
#include "esphome/core/log.h"
#include "M5Unified.h"

namespace esphome {
namespace papertouch {

static const char *const TAG = "papertouch";

void PaperTouch::setup() {
  ESP_LOGI(TAG, "Setting up PaperTouch...");

  // Init M5.Touch
  if (!M5.Touch.isEnabled()) {
    ESP_LOGW(TAG, "M5.Touch not enabled or GT911 not found!");
  } else {
    ESP_LOGI(TAG, "M5.Touch initialized successfully.");
  }
}

void PaperTouch::loop() {
  M5.update();
}

void PaperTouch::update_touches() {
  this->touches_.clear();

  if (!M5.Touch.isEnabled()) {
    return;
  }

  uint16_t x, y;
  if (M5.Touch.getCount() > 0) {  // Assuming getCount() works
    if (M5.Touch.getTouch(0, &x, &y)) {  // Retrieve touch point at index 0
      touchscreen::TouchPoint point;
      point.x = x;
      point.y = y;
      this->touches_[0] = point;  // Replace push_back with setting map element
    }
  }
}

}  // namespace papertouch
}  // namespace esphome
