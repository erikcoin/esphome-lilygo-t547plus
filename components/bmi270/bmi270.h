#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace bmi270_sensor {

class BMI270Sensor : public PollingComponent {
 public:
  void setup() override;
  void update() override;

  void set_accel_x_sensor(sensor::Sensor *s) { this->accel_x_sensor_ = s; }
  void set_accel_y_sensor(sensor::Sensor *s) { this->accel_y_sensor_ = s; }
  void set_accel_z_sensor(sensor::Sensor *s) { this->accel_z_sensor_ = s; }

  void set_gyro_x_sensor(sensor::Sensor *s) { this->gyro_x_sensor_ = s; }
  void set_gyro_y_sensor(sensor::Sensor *s) { this->gyro_y_sensor_ = s; }
  void set_gyro_z_sensor(sensor::Sensor *s) { this->gyro_z_sensor_ = s; }

 protected:
  sensor::Sensor *accel_x_sensor_{nullptr};
  sensor::Sensor *accel_y_sensor_{nullptr};
  sensor::Sensor *accel_z_sensor_{nullptr};

  sensor::Sensor *gyro_x_sensor_{nullptr};
  sensor::Sensor *gyro_y_sensor_{nullptr};
  sensor::Sensor *gyro_z_sensor_{nullptr};
};

}  // namespace bmi270_sensor
}  // namespace esphome
