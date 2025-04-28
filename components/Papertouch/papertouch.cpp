#include "papertouch.h"
#include "esphome/core/log.h"
#include "M5Unified.h"
#include "M5GFX"

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

  // Check if there are any active touch points
  int touch_count = M5.Touch.getCount();
  if (touch_count > 0) {
    uint16_t x, y;

    // Use getPoint() to get the touch coordinates for the first touch
    if (M5.Touch.getPressPoint(&x, &y)) {
      touchscreen::TouchPoint point;
      point.x = x;
      point.y = y;
      this->touches_.push_back(point);  // Store the touch point
    }
  }
}


}  // namespace papertouch
}  // namespace esphome
