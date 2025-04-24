#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/display/display.h"

#include <M5Unified.h>
#include <M5GFX.h>

namespace esphome {
namespace m5papers3_display_m5gfx {

class M5PaperS3DisplayM5GFX : public display::Display {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void update() override;
  void fill(Color color) override;
  int get_width_internal() override;
  int get_height_internal() override;
  display::DisplayType get_display_type() override {
    return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
  }

  void set_rotation(int rotation);
  void set_writer(std::function<void(display::Display &)> &&writer) { this->writer_ = writer; }

  void draw_pixel_at(int x, int y, esphome::Color color) override;

 protected:
  void draw_absolute_pixel_internal(int x, int y, esphome::Color color);

  int rotation_{0};
  std::function<void(display::Display &)> writer_{nullptr};

  lgfx::LGFX_Sprite canvas_{&M5.Display};
  uint8_t get_native_m5gfx_color_(esphome::Color color);
};

}  // namespace m5papers3_display_m5gfx
}  // namespace esphome
