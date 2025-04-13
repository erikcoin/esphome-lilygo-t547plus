#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/api.h"
#include <M5Unified.h>

namespace esphome {
namespace erik {

class ErikDisplay : public display::DisplayBuffer, public PollingComponent {
 public:
  void set_light_state(text_sensor::TextSensor *state) { this->light_state_ = state; }

  void setup() override;
  void update() override;
  void draw_display() override;

  int get_width() override { return 540; }
  int get_height() override { return 960; }

 protected:
  text_sensor::TextSensor *light_state_{nullptr};

  void draw_button_(bool state);
  void toggle_light_();
};

}  // namespace erik
}  // namespace esphome
