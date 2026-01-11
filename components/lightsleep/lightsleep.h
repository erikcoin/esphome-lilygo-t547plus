
#pragma once

#include "esphome/core/component.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include <M5Unified.h>

namespace esphome {
namespace lightsleep {

class LightSleepComponent : public Component {
 public:
  void set_wakeup_pin(int pin) { wakeup_pin_ = pin; }
  void set_wake_on_touch(bool v) { wake_on_touch_ = v; }
  void set_turn_off_display(bool v) { turn_off_display_ = v; }
  void set_min_inactive_time(uint32_t ms) { min_inactive_time_ = ms; }
  void set_wake_every(uint32_t ms) { wake_every_ = ms; }

  void setup() override;
  void loop() override;

 protected:
  int wakeup_pin_{-1};
  bool wake_on_touch_{true};
  bool turn_off_display_{false};

  uint32_t min_inactive_time_{30000};
  uint32_t wake_every_{0};

  uint32_t last_activity_{0};
  uint32_t last_wake_timer_{0};

  void enter_light_sleep_();
};

}  // namespace lightsleep
}  // namespace esphome
