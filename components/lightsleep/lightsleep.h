#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"

#ifdef USE_ESP32
#include <esp_sleep.h>
#include <esp_wifi.h>
#include <driver/gpio.h>
#include <driver/rtc_io.h>

namespace esphome {
namespace light_sleep {

class LightSleepComponent : public Component {
 public:
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

  void set_sleep_duration(uint32_t duration_ms) { sleep_duration_ms_ = duration_ms; }
  void set_run_duration(uint32_t duration_ms) { run_duration_ms_ = duration_ms; }
  void set_wakeup_pin(InternalGPIOPin *pin) { wakeup_pin_ = pin; }
  void set_wakeup_level(int level) { wakeup_level_ = level; }

 protected:
  void prepare_for_sleep_();
  void restore_after_sleep_();
  
  uint32_t sleep_duration_ms_{300000};  // 5 minutes default
  uint32_t run_duration_ms_{60000};     // 1 minute default
  InternalGPIOPin *wakeup_pin_{nullptr};
  int wakeup_level_{0};  // 0 = LOW, 1 = HIGH
  bool gpio_configured_{false};
  uint32_t last_wakeup_time_{0};
  bool sleeping_{false};
  bool wifi_was_enabled_{false};
};

}  // namespace light_sleep
}  // namespace esphome
