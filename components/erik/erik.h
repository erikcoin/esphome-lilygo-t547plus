#pragma once

#include "esphome/core/component.h"
#include <epdiy.h>
#include <M5Unified.h>
#include <functional>

namespace esphome {
namespace erik {

class ErikDisplay : public PollingComponent {
 public:
  ErikDisplay() : PollingComponent(1000) {}  // Default update interval: 1000ms

  void setup() override;
  void update() override;

  void set_touch_callback(std::function<void()> &&callback) {
    this->touch_callback_ = std::move(callback);
  }

  void show_state(bool on);

 protected:
  bool last_touch_state_ = false;
  bool light_on_ = false;
  std::function<void()> touch_callback_;

  void draw_screen_();
  bool is_touch_inside_button_();
};

}  // namespace erik
}  // namespace esphome
