#pragma once

#include "esphome/core/component.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include <M5Unified.h>

namespace esphome {
namespace lightsleep {

class LightSleepComponent : public Component {
 public:
  void set_wakeup_pin(int pin) { this->wakeup_pin_ = pin; }
  void set_wakeup_minutes(uint32_t minutes) { this->wakeup_minutes_ = minutes; }

  void setup() override;
  void loop() override;

 protected:
  int wakeup_pin_{-1};
  uint32_t wakeup_minutes_{0};
  uint32_t last_wake_time_{0};
};

}  // namespace lightsleep
}  // namespace esphome
