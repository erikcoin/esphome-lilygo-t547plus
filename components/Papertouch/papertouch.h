#pragma once

#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/core/component.h"

namespace esphome {
namespace papertouch {

class PaperTouch : public touchscreen::Touchscreen {
 public:
  void setup() override;
  void loop() override;
  void update_touches() override;
};

}  // namespace papertouch
}  // namespace esphome
