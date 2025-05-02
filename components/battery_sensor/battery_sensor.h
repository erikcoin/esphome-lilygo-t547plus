#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include <M5Unified.h>

namespace esphome {
namespace battery_sensor {

class MyBatterySensor : public sensor::Sensor, public PollingComponent {
 public:
  MyBatterySensor() : PollingComponent(60000) {}

  void update() override {
    float voltage = M5.Power.getBatteryVoltage() / 1000.0;  // millivolt naar volt
    ESP_LOGD("battery", "Battery voltage: %.2f V", voltage);
    publish_state(voltage);
  }
};

class MyBatteryPercentageSensor : public sensor::Sensor, public PollingComponent {
 public:
  MyBatteryPercentageSensor() : PollingComponent(60000) {}

  void update() override {
    float voltage = M5.Power.getBatteryVoltage() / 1000.0;
    float percent = (voltage - 3.0f) / (4.2f - 3.0f) * 100.0f;
    percent = std::max(0.0f, std::min(100.0f, percent));  // Clamp 0–100%
    ESP_LOGD("battery", "Battery percentage: %.0f %%", percent);
    publish_state(percent);
  }
};

}  // namespace battery_sensor
}  // namespace esphome
