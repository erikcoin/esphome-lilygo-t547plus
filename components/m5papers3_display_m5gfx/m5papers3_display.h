#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/display/display.h"

// Includes voor M5GFX en M5Unified
#include <M5Unified.h> // Voor M5.begin() en M5.Display object
#include <M5GFX.h>     // Voor LGFX en M5GFX types

// !! BELANGRIJK: LGFX_Sprite zit in de lgfx namespace !!
//namespace lgfx { using LGFX_Sprite = ::LGFX_Sprite; } // Breng LGFX_Sprite in de lgfx namespace als het nog niet zo is

namespace esphome {
namespace m5papers3_display_m5gfx {

// !! Verwijder PollingComponent uit de inheritance list !!
class M5PaperS3DisplayM5GFX : public display::Display {
 public:
  // Standaard Component methodes (setup, dump_config, get_setup_priority blijven hetzelfde)
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  // PollingComponent methode (update komt via display::Display)
  void update() override;

  // Display methodes (fill, get_width/height_internal, get_display_type blijven hetzelfde)
  void fill(Color color) override;
  int get_width_internal() override;
  int get_height_internal() override;
  display::DisplayType get_display_type() override {
    return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
  }

  // Configuratie setters (blijven hetzelfde)
  void set_rotation(int rotation);
  void set_writer(std::function<void(display::Display &)> &&writer) { 
   ESP_LOGD("display", "set_writer() called");
   this->writer_ = writer; }
 
  void draw_pixel_at(int x, int y, esphome::Color color) override;

 protected: // !! Verplaats draw_absolute_pixel_internal naar protected !!
  void draw_absolute_pixel_internal(int x, int y, esphome::Color color) ;
  // Helper: Converteer ESPHome Color naar M5GFX kleur formaat (blijft hetzelfde)
  uint32_t get_native_m5gfx_color_(Color color);

  // Member variabelen (rotation, writer blijven hetzelfde)
  int rotation_{0};
  std::function<void(display::Display &)> writer_{nullptr};

  // !! Gebruik lgfx::LGFX_Sprite voor de canvas !!
  M5Canvas canvas_;
//lgfx::LGFX_Sprite canvas_;
};

} // namespace m5papers3_display_m5gfx
} // namespace esphome
