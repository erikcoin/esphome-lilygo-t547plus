#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/display/display.h"

// Includes voor M5GFX en M5Unified
#include <M5Unified.h> // Voor M5.begin() en M5.Display object
#include <M5GFX.h>     // Voor LGFX en M5GFX types

namespace esphome {
namespace m5papers3_display_m5gfx {

// Gebruik de M5GFX sprite class
using M5Sprite = M5GFX::LGFX_Sprite; // Alias voor leesbaarheid

class M5PaperS3DisplayM5GFX : public PollingComponent, public display::Display {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  void update() override; // Van PollingComponent

  // Display overrides
  void fill(Color color) override;
  int get_width_internal() override;
  int get_height_internal() override;
  display::DisplayType get_display_type() override {
    // Blijft grayscale, ook al gebruikt M5GFX intern mogelijk RGB formats
    return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
  }

  // Configuratie setters
  void set_rotation(int rotation);
  void set_writer(std::function<void(display::Display &)> &&writer) { this->writer_ = writer; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;

  // Helper: Converteer ESPHome Color naar M5GFX kleur formaat (bv. uint16_t of uint32_t)
  // Laten we uint32_t gebruiken (bv. voor color888).
  uint32_t get_native_m5gfx_color_(Color color);

  int rotation_{0}; // ESPHome rotatie (0, 90, 180, 270)
  std::function<void(display::Display &)> writer_{nullptr};

  // M5GFX Sprite (canvas) om op te tekenen
  M5Sprite canvas_;
};

} // namespace m5papers3_display_m5gfx
} // namespace esphome
