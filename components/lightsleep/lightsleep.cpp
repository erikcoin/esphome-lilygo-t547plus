#include "lightsleep.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

#include "esp_sleep.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"


namespace esphome {
namespace light_sleep {

static const char *const TAG = "light_sleep";

void LightSleepComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up Light Sleep...");
 ;
  // Configure timer wake-up
  esp_sleep_enable_timer_wakeup(this->sleep_duration_ms_ * 1000ULL);
  ESP_LOGCONFIG(TAG, "  Timer wake-up: %u ms", this->sleep_duration_ms_);
  ESP_LOGCONFIG(TAG, "  Run duration: %u ms", this->run_duration_ms_);
  
  // Configure GPIO wake-up if pin is set
//  if (this->wakeup_pin_ != nullptr) {
    pin = 48;
    gpio_num_t gpio_num = (gpio_num_t) this->wakeup_pin_->get_pin();
    
   // #if defined(USE_ESP32_VARIANT_ESP32S2) || defined(USE_ESP32_VARIANT_ESP32S3)
      // ESP32-S2 and S3 support GPIO wakeup in light sleep
      if (this->wakeup_level_ == 0) {
        esp_sleep_enable_gpio_wakeup();
        ESP_ERROR_CHECK(gpio_wakeup_enable(gpio_num, GPIO_INTR_LOW_LEVEL));
      } else {
        esp_sleep_enable_gpio_wakeup();
        ESP_ERROR_CHECK(gpio_wakeup_enable(gpio_num, GPIO_INTR_HIGH_LEVEL));
      }
      
      // Configure pin as input with pull-up/pull-down
      //gpio_config_t config;
      //config.pin_bit_mask = (1ULL << gpio_num);
     /// config.mode = GPIO_MODE_INPUT;
  //    config.intr_type = GPIO_INTR_DISABLE;
 //     
 //     if (this->wakeup_level_ == 0) {
 //       config.pull_up_en = GPIO_PULLUP_ENABLE;
 //       config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  //    } else {
  //      config.pull_up_en = GPIO_PULLUP_DISABLE;
  //      config.pull_down_en = GPIO_PULLDOWN_ENABLE;
  //    }
      
    //  gpio_config(&config);
      
      this->gpio_configured_ = true;
      ESP_LOGD(TAG, "  GPIO wake-up: GPIO%d (level: %s)", gpio_num, 
                    this->wakeup_level_ ? "HIGH" : "LOW");
//    #else
      // For ESP32 (original), use ext0 wakeup
//      esp_sleep_enable_ext0_wakeup(gpio_num, this->wakeup_level_);
      
      // Configure RTC GPIO
//      rtc_gpio_init(gpio_num);
//      rtc_gpio_set_direction(gpio_num, RTC_GPIO_MODE_INPUT_ONLY);
      
//      if (this->wakeup_level_ == 0) {
//        rtc_gpio_pullup_en(gpio_num);
//        rtc_gpio_pulldown_dis(gpio_num);
//      } else {
//        rtc_gpio_pulldown_en(gpio_num);
//        rtc_gpio_pullup_dis(gpio_num);
//      }
      
//      this->gpio_configured_ = true;
//      ESP_LOGCONFIG(TAG, "  EXT0 wake-up: GPIO%d (level: %s)", gpio_num, 
//                    this->wakeup_level_ ? "HIGH" : "LOW");
//    #endif
 // }
  
  // Record initial boot time
  this->last_wakeup_time_ = millis();
}

void LightSleepComponent::prepare_for_sleep_() {
  ESP_LOGI(TAG, "Preparing for sleep...");
  
  // Disable WiFi to save power during sleep
  #ifdef USE_WIFI
  wifi_mode_t mode;
  esp_err_t err = esp_wifi_get_mode(&mode);
  if (err == ESP_OK && mode != WIFI_MODE_NULL) {
    this->wifi_was_enabled_ = true;
    ESP_LOGI(TAG, "  Stopping WiFi...");
    esp_wifi_stop();
    delay(10); // Give WiFi time to stop gracefully
  } else {
    this->wifi_was_enabled_ = false;
  }
  #endif
  
  // Flush all pending logs before sleeping
  ESP_LOGI(TAG, "  Flushing logs...");
  esp_log_level_set("*", ESP_LOG_NONE);
  delay(50);
  
  // Allow other components to finish their work
  App.loop();
}

void LightSleepComponent::restore_after_sleep_() {
  ESP_LOGI(TAG, "Restoring after sleep...");
  
  // Re-enable logging
  esp_log_level_set("*", ESP_LOG_DEBUG);
  
  // Restart WiFi if it was enabled before sleep
  #ifdef USE_WIFI
  if (this->wifi_was_enabled_) {
    ESP_LOGI(TAG, "  Restarting WiFi...");
    esp_wifi_start();
    delay(10); // Give WiFi time to start
  }
  #endif
  
  // Give components time to reinitialize
  delay(100);
  App.loop();
}

void LightSleepComponent::loop() {
  // Check if we should enter sleep
  uint32_t current_time = millis();
  uint32_t time_awake = current_time - this->last_wakeup_time_;
  
  // Only enter sleep if we've been awake for the configured duration
  if (time_awake >= this->run_duration_ms_ && !this->sleeping_) {
    this->sleeping_ = true;
    
    // Log wake-up cause from previous sleep
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    switch (wakeup_reason) {
      case ESP_SLEEP_WAKEUP_GPIO:
        ESP_LOGI(TAG, "Previous wake: GPIO interrupt");
        break;
      case ESP_SLEEP_WAKEUP_EXT0:
        ESP_LOGI(TAG, "Previous wake: EXT0 (GPIO)");
        break;
      case ESP_SLEEP_WAKEUP_TIMER:
        ESP_LOGI(TAG, "Previous wake: Timer");
        break;
      case ESP_SLEEP_WAKEUP_UNDEFINED:
        ESP_LOGD(TAG, "Initial boot");
        break;
      default:
        ESP_LOGD(TAG, "Previous wake: Other source (%d)", wakeup_reason);
        break;
    }
    
    // Prepare system for sleep
    this->prepare_for_sleep_();
    
    ESP_LOGI(TAG, "Entering light sleep (awake time: %u ms)...", time_awake);
    
    // Enter light sleep - this is the actual sleep call
    int64_t sleep_start = esp_timer_get_time();
    esp_err_t err = esp_light_sleep_start();
    int64_t sleep_duration = (esp_timer_get_time() - sleep_start) / 1000;
    
    if (err == ESP_OK) {
      ESP_LOGI(TAG, "Woke up after %lld ms", sleep_duration);
    } else {
      ESP_LOGW(TAG, "Light sleep failed: %d", err);
    }
    
    // Restore system after sleep
    this->restore_after_sleep_();

    // Reset the timer and state
    this->last_wakeup_time_ = millis();
    this->sleeping_ = false;
  }
}

}  // namespace light_sleep
}  // namespace esphome
