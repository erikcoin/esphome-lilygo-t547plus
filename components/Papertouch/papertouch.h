#pragma once

#include "esphome.h"
#include <M5Unified.h>

namespace esphome {
namespace papertouch {

class Papertouch : public Component {
 public:
  void setup() override {
    M5.begin();  // Initialize the M5Stack device
    M5.Touch.begin();  // Initialize the touch screen
  }

  void update() override {
    if (M5.Touch.isPressed()) {  // Check if the screen is pressed
      TouchPoint touch = M5.Touch.getPressPoint();  // Get the touch point
      ESP_LOGD("papertouch", "Touch detected at: %d, %d", touch.x, touch.y);
      
      // Do something with the touch coordinates
      // For example, you could trigger actions or send data to other parts of the system
    }
  }
};

}  // namespace papertouch
}  // namespace esphome
