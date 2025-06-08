#include "bmi270.h"
#include <M5Unified.h>

namespace esphome {
namespace bmi270_sensor {

void BMI270Sensor::setup() {
  // BMI270 is al geïnitialiseerd door M5Unified
}

void BMI270Sensor::update() {
  M5.Imu.update();

  float ax, ay, az;
  float gx, gy, gz;

  if (M5.Imu.getAccel(&ax, &ay, &az)) {
    if (this->accel_x_sensor_ != nullptr) this->accel_x_sensor_->publish_state(ax);
    if (this->accel_y_sensor_ != nullptr) this->accel_y_sensor_->publish_state(ay);
    if (this->accel_z_sensor_ != nullptr) this->accel_z_sensor_->publish_state(az);
  }

  if (M5.Imu.getGyro(&gx, &gy, &gz)) {
    if (this->gyro_x_sensor_ != nullptr) this->gyro_x_sensor_->publish_state(gx);
    if (this->gyro_y_sensor_ != nullptr) this->gyro_y_sensor_->publish_state(gy);
    if (this->gyro_z_sensor_ != nullptr) this->gyro_z_sensor_->publish_state(gz);
  }
}

}  // namespace bmi270_sensor
}  // namespace esphome
