#pragma once

#include "esphome/core/component.h"
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/display/display_color_utils.h"
#include <epdiy.h>
#include <M5GFX.h>

namespace esphome {
namespace erik {


class Erik : public display::DisplayBuffer {

 public:
  void set_greyscale(bool greyscale) {
    this->greyscale_ = greyscale;
  }

  float get_setup_priority() const override;

  void dump_config() override;

  void display();
  void clean();
  void update() override;

  void setup() override;

  uint8_t get_panel_state() { return this->panel_on_; }
  bool get_greyscale() { return this->greyscale_; }


  display::DisplayType get_display_type() override {
    return get_greyscale() ? display::DisplayType::DISPLAY_TYPE_GRAYSCALE : display::DisplayType::DISPLAY_TYPE_BINARY;
  }


  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  int get_width() override { return 960; }
  int get_height() override { return 540; }


 protected:
  M5GFX display_;
};

}  // namespace erik
}  // namespace esphome
