#include "lightsleep.h"
#include "esp_sleep.h"

namespace esphome {
namespace lightsleep {

static const char *TAG = "lightsleep";

void LightSleepComponent::enter_light_sleep_() {
  ESP_LOGI(TAG, "Preparing to enter light sleep (wake_on_touch=%d, turn_off_display=%d, wake_every_ms=%u)",
           (int)wake_on_touch_, (int)turn_off_display_, (unsigned)wake_every_);

  // Give touch IRQs a chance to clear so we don't immediately wake on a stale IRQ
  for (int i = 0; i < 3; i++) {
    M5.update();
    lgfx::v1:delay(50);
  }

  if (turn_off_display_) {
    ESP_LOGI(TAG, "Putting display to sleep");
    M5.Display.sleep();
    lgfx::v1:delay(10);
  }

  // Clear previously configured wake sources to start fresh
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
  // Note: there is no esp_sleep_disable_wakeup_source for GPIO specifically on older IDs,
  // but we will explicitly disable gpio wake after waking.

  // Configure GPIO touch wake if requested and pin valid
  if (wake_on_touch_ && wakeup_pin_ >= 0) {
    ESP_LOGI(TAG, "Enabling GPIO wake on pin %d (LOW level)", wakeup_pin_);
    gpio_wakeup_enable((gpio_num_t)wakeup_pin_, GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
  }

  // Configure timer wake if requested (wake_every_ is in milliseconds)
  if (wake_every_ > 0) {
    // esp_sleep_enable_timer_wakeup expects microseconds
    uint64_t us = (uint64_t)wake_every_ * 1000ULL;
    ESP_LOGI(TAG, "Enabling timer wakeup in %llu us (%u ms)", (unsigned long long)us, (unsigned)wake_every_);
    esp_sleep_enable_timer_wakeup(us);
  }

  ESP_LOGI(TAG, "Entering light sleep now...");
  esp_light_sleep_start();

  // Execution resumes here after wake
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  switch (cause) {
    case ESP_SLEEP_WAKEUP_TIMER:
      ESP_LOGI(TAG, "Woke up by TIMER");
      break;
    case ESP_SLEEP_WAKEUP_GPIO:
      ESP_LOGI(TAG, "Woke up by GPIO");
      break;
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    default:
      ESP_LOGI(TAG, "Woke up by OTHER/UNDEFINED reason: %d", (int)cause);
      break;
  }

  // Cleanup: disable gpio wake if we enabled it
  if (wakeup_pin_ >= 0) {
    gpio_wakeup_disable((gpio_num_t)wakeup_pin_);
  }
  // Disable timer wake to avoid lingering
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);

  // Restore display if needed
  if (turn_off_display_) {
    lgfx::v1:delay(10);
    M5.Display.wakeup();
  }

  // small delay to settle
  lgfx::v1:delay(50);
}

}  // namespace lightsleep
}  // namespace esphome
