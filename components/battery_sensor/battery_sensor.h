#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include <M5Unified.h>

namespace esphome {
namespace battery_sensor  {

class MyBatterySensor : public sensor::Sensor, public PollingComponent {
 public:
  MyBatterySensor() : PollingComponent(60000) {}  // elke 60s updaten

  void setup() override {
    // Zorg dat M5 al geïnitialiseerd is, dus M5.begin() moet al elders zijn aangeroepen
  }

  void update() override {
    float voltage = M5.Power.getBatteryVoltage();  // in volt
    ESP_LOGD("battery", "Battery voltage: %.2f V", voltage);
    publish_state(voltage);
  }
};

}  // namespace my_battery
}  // namespace esphome
