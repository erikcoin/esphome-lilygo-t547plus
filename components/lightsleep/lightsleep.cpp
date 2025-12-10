#include "lightsleep.h"

namespace esphome {
namespace lightsleep {

void LightSleepComponent::setup() {
  M5.begin();
  this->last_wake_time_ = millis();
}

void LightSleepComponent::loop() {
  // Wakeup timer
  if (this->wakeup_minutes_ > 0) {
    uint32_t interval = this->wakeup_minutes_ * 60 * 1000;
    if ((millis() - this->last_wake_time_) > interval) {
      this->last_wake_time_ = millis();
      esp_light_sleep_start();
      return;
    }
  }

  // Touch wakeup or GPIO wakeup
  if (this->wakeup_pin_ >= 0) {
    gpio_wakeup_enable((gpio_num_t)this->wakeup_pin_, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
    esp_light_sleep_start();
    gpio_wakeup_disable((gpio_num_t)this->wakeup_pin_);
  }
}

}  // namespace lightsleep
}  // namespace esphome
