#include "lightsleep.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_time.h"
namespace esphome {
namespace lightsleep {

static const char *TAG = "lightsleep";
void LightSleepComponent::setup() {
  // Initialize timers
  last_activity_ = millis();
  last_wake_timer_ = millis();
}

void LightSleepComponent::loop() {
  uint32_t now = millis();

  // Trigger sleep if inactive too long
  if (min_inactive_time_ > 0 && (now - last_activity_) > min_inactive_time_) {
    enter_light_sleep_();
    last_activity_ = millis();
    last_wake_timer_ = millis();
  }

  // Trigger periodic sleep if configured
  if (wake_every_ > 0 && (now - last_wake_timer_) >= wake_every_) {
    enter_light_sleep_();
    last_wake_timer_ = millis();
  }
}

void LightSleepComponent::enter_light_sleep_() {
  ESP_LOGI(TAG, "Preparing to enter light sleep (wake_on_touch=%d, turn_off_display=%d, wake_every_ms=%u)",
           (int)wake_on_touch_, (int)turn_off_display_, (unsigned)wake_every_);

  // Give touch IRQs a chance to clear so we don't immediately wake on a stale IRQ
  for (int i = 0; i < 3; i++) {
//    M5.update();
    delay(50);
  }

  if (turn_off_display_) {
    ESP_LOGI(TAG, "Putting display to sleep");
 //   M5.Display.sleep();
    delay(10);
  }

  // Clear previously configured wake sources to start fresh
  //esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);
  // Note: there is no esp_sleep_disable_wakeup_source for GPIO specifically on older IDs,
  // but we will explicitly disable gpio wake after waking.
  wakeup_pin_ = 48;
  // Configure GPIO touch wake if requested and pin valid
 // if (wake_on_touch_ && wakeup_pin_ >= 0) {
  if (wake_on_touch_ > 0) {
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

// Shut down WiFi safely before sleep (IDF API)
// ---------------------------------------------
wifi_mode_t current_mode;
esp_wifi_get_mode(&current_mode);

if (current_mode != WIFI_MODE_NULL) {
  ESP_LOGI(TAG, "Stopping WiFi before sleep");
  esp_wifi_stop();         // stops STA/AP
  esp_wifi_set_mode(WIFI_MODE_NULL);
  // tiny delay to settle
  vTaskDelay(pdMS_TO_TICKS(50));
}
  
  ESP_LOGI(TAG, "Entering light sleep now...");
  esp_light_sleep_start();
   // Execution resumes here after wake

  // Restore WiFi after wake (IDF API)
// ---------------------------------------------
ESP_LOGI(TAG, "Restarting WiFi after wake");

esp_wifi_set_mode(WIFI_MODE_STA);   // we want normal station mode
esp_wifi_start();                   // bring WiFi up

// Let ESPHome reconnect the network
vTaskDelay(pdMS_TO_TICKS(200));
  
  // Reset inactivity timers on wake because millis() jumps backward
     last_activity_ = esp_timer_get_time() / 1000ULL;
     last_wake_timer_ = last_activity_;

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
 // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_TIMER);

  // Restore display if needed
  if (turn_off_display_) {
    delay(10);
 //   M5.Display.wakeup();
  }

  // small delay to settle
  delay(50);
}

}  // namespace lightsleep
}  // namespace esphome
