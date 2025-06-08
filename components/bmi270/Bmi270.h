#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"
#include <M5Unified.h>

namespace esphome {
namespace bmi270_sensor {

class BMI270Sensor : public PollingComponent {
 public:
  sensor::Sensor *accel_x_sensor{nullptr};
  sensor::Sensor *accel_y_sensor{nullptr};
  sensor::Sensor *accel_z_sensor{nullptr};
  sensor::Sensor *gyro_x_sensor{nullptr};
  sensor::Sensor *gyro_y_sensor{nullptr};
  sensor::Sensor *gyro_z_sensor{nullptr};

  void setup() override {
    M5.begin();
    M5.Imu.begin();
  }

  void update() override {
    float ax, ay, az;
    float gx, gy, gz;

    if (M5.Imu.getAccel(&ax, &ay, &az)) {
      if (accel_x_sensor) accel_x_sensor->publish_state(ax);
      if (accel_y_sensor) accel_y_sensor->publish_state(ay);
      if (accel_z_sensor) accel_z_sensor->publish_state(az);
    }

    if (M5.Imu.getGyro(&gx, &gy, &gz)) {
      if (gyro_x_sensor) gyro_x_sensor->publish_state(gx);
      if (gyro_y_sensor) gyro_y_sensor->publish_state(gy);
      if (gyro_z_sensor) gyro_z_sensor->publish_state(gz);
    }
  }
};

}  // namespace bmi270_sensor
}  // namespace esphome
