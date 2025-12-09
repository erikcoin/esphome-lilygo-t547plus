#pragma once

#include "esphome.h"
#include <M5Unified.h>
#include <driver/gpio.h>
#include <esp_sleep.h>

namespace esphome {
namespace m5p3_light_sleep {

class M5P3LightSleep : public Component {
 public:
  bool wake_on_touch_{true};
  bool turn_off_display_{true};
  bool verbose_{false};
  uint32_t min_inactive_ms_{30000};

  uint32_t last_activity_{0};
  Trigger<> *on_wake_trigger_ = new Trigger<>();

  void set_wake_on_touch(bool v) { wake_on_touch_ = v; }
  void set_turn_off_display(bool v) { turn_off_display_ = v; }
  void set_verbose(bool v) { verbose_ = v; }
  void set_min_inactive_time(uint32_t ms) { min_inactive_ms_ = ms; }

  Trigger<> *get_on_wake_trigger() { return on_wake_trigger_; }

  void setup() override {
    if (verbose_)
      ESP_LOGI("m5p3_light_sleep", "Starting light sleep component...");

    auto cfg = M5.config();
    cfg.internal_imu = false;
    M5.begin(cfg);

    last_activity_ = millis();
  }

  void loop() override {
    M5.update();
    if (M5.Touch.getCount() > 0) {
      last_activity_ = millis();
    }

    if (millis() - last_activity_ > min_inactive_ms_) {
      if (verbose_)
        ESP_LOGI("m5p3_light_sleep", "Auto-sleep triggered.");
      enter_sleep_();
      last_activity_ = millis();
    }
  }

  void enter_sleep_() {
    if (verbose_)
      ESP_LOGI("m5p3_light_sleep", "Entering light sleep...");

    if (turn_off_display_) {
      M5.Display.sleep();
    }

    M5.Power.setLed(255);

    if (wake_on_touch_) {
      gpio_wakeup_enable((gpio_num_t)48, GPIO_INTR_LOW_LEVEL);
      esp_sleep_enable_gpio_wakeup();
    }

    esp_light_sleep_start();

    gpio_wakeup_disable((gpio_num_t)48);

    if (verbose_)
      ESP_LOGI("m5p3_light_sleep", "Woke up!");

    M5.Power.setLed(0);
    M5.Display.wakeup();

    on_wake_trigger_->trigger();
  }

  // Action callable from YAML
  void sleep_now() { enter_sleep_(); }
};


// ---------------- ACTION WRAPPER ---------------------

class SleepNowAction : public Action {
 public:
  M5P3LightSleep *component_;

  void set_component(M5P3LightSleep *c) { component_ = c; }

  void play(ActionContext ctx) override {
    if (component_) {
      component_->sleep_now();
    }
    ctx.finish(true);
  }
};

}  // namespace m5p3_light_sleep
}  // namespace esphome
