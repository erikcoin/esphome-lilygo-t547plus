#include "bmi270.h"
#include <M5Unified.h>

namespace esphome {
namespace bmi270_sensor {

void BMI270Sensor::setup() {
  // BMI270 is automatisch geinitialiseerd door M5Unified
}

void BMI270Sensor::update() {
  M5.Imu.update();

  if (this->accel_x_sensor_ != nullptr)
    this->accel_x_sensor_->publish_state(M5.Imu.accel.x);
  if (this->accel_y_sensor_ != nullptr)
    this->accel_y_sensor_->publish_state(M5.Imu.accel.y);
  if (this->accel_z_sensor_ != nullptr)
    this->accel_z_sensor_->publish_state(M5.Imu.accel.z);

  if (this->gyro_x_sensor_ != nullptr)
    this->gyro_x_sensor_->publish_state(M5.Imu.gyro.x);
  if (this->gyro_y_sensor_ != nullptr)
    this->gyro_y_sensor_->publish_state(M5.Imu.gyro.y);
  if (this->gyro_z_sensor_ != nullptr)
    this->gyro_z_sensor_->publish_state(M5.Imu.gyro.z);
}

}  // namespace bmi270_sensor
}  // namespace esphome
