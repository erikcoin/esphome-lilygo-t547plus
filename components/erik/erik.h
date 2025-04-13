#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include <epdiy.h>
#include <M5Unified.h>
#include <functional>

namespace esphome {
namespace erik {

class ErikDisplay : public display::DisplayBuffer {
 public:
  void setup() override;
  void update() override;
  void draw(display::DisplayBuffer &it) override;

  int get_width() override { return 540; }
  int get_height() override { return 960; }

  void set_touch_callback(std::function<void()> &&callback) {
    this->touch_callback_ = std::move(callback);
  }

 protected:
  bool last_touch_state_ = false;
  std::function<void()> touch_callback_;
};

}  // namespace erik
}  // namespace esphome
