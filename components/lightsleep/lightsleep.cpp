#include "lightsleep.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace lightsleep {

void LightSleepComponent::setup() {
  M5.begin();
  last_activity_ = lgfx::v1::millis();
  last_wake_timer_ = millis();
}

void LightSleepComponent::loop() {
  M5.update();

  // Check for activity (touch)
  if (M5.Touch.isEnabled() && M5.Touch.getCount() > 0) {
    last_activity_ = millis();
  }

  const uint32_t now = millis();

  // 1️⃣ Periodic wake-up timer
  if (wake_every_ > 0 && (now - last_wake_timer_) >= wake_every_) {
    last_wake_timer_ = now;
    enter_light_sleep_();
    return;
  }

  // 2️⃣ Inactivity timeout sleep
  if ((now - last_activity_) >= min_inactive_time_) {
    enter_light_sleep_();
    last_activity_ = now;
  }
}

void LightSleepComponent::enter_light_sleep_() {
  if (turn_off_display_) {
    M5.Display.sleep();
  }

  if (wake_on_touch_ && wakeup_pin_ >= 0) {
    gpio_wakeup_enable((gpio_num_t) wakeup_pin_, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
  }

  esp_light_sleep_start();

  if (wakeup_pin_ >= 0)
    gpio_wakeup_disable((gpio_num_t) wakeup_pin_);

  if (turn_off_display_) {
    M5.Display.wakeup();
  }
}

}  // namespace lightsleep
}  // namespace esphome
