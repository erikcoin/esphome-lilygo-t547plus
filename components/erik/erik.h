#pragma once

#include "esphome/core/component.h"
#include <epdiy.h>
#include <M5Unified.h>

namespace esphome {
namespace erik {

class ErikComponent : public PollingComponent {
 public:
  void set_text(const std::string &text) { text_ = text; }
  void set_x(int x) { x_ = x; }
  void set_y(int y) { y_ = y; }
  void set_size(int size) { size_ = size; }

  void setup() override;
  void update() override;

 protected:
  M5GFX display_;
  std::string text_;
  int x_ = 10;
  int y_ = 10;
  int size_ = 2;
};

}  // namespace erik
}  // namespace esphome
