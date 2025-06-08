#include "bmi270.h"
#include <M5Unified.h>

namespace esphome {
namespace bmi270_sensor {

void BMI270Sensor::setup() {
  // BMI270 is automatisch geïnitialiseerd door M5Unified
}

void BMI270Sensor::update() {
  M5.Imu.update();

  m5::imu_raw_value_t accel;
  m5::imu_raw_value_t gyro;

  if (M5.Imu.getAccel(&accel)) {
    if (this->accel_x_sensor_ != nullptr)
      this->accel_x_sensor_->publish_state(accel.x);
    if (this->accel_y_sensor_ != nullptr)
      this->accel_y_sensor_->publish_state(accel.y);
    if (this->accel_z_sensor_ != nullptr)
      this->accel_z_sensor_->publish_state(accel.z);
  }

  if (M5.Imu.getGyro(&gyro)) {
    if (this->gyro_x_sensor_ != nullptr)
      this->gyro_x_sensor_->publish_state(gyro.x);
    if (this->gyro_y_sensor_ != nullptr)
      this->gyro_y_sensor_->publish_state(gyro.y);
    if (this->gyro_z_sensor_ != nullptr)
      this->gyro_z_sensor_->publish_state(gyro.z);
  }
}

}  // namespace bmi270_sensor
}  // namespace esphome
