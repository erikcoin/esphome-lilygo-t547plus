#pragma once

#include "esphome/components/display/display_buffer.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"

#include <M5Unified.h>
#include <M5GFX.h>


namespace esphome {
namespace m5papers3ns {

class M5PaperS3Display : public esphome::display::Display, public esphome::Component {
 public:
  void setup() override;
  void loop() override { M5.update(); }
  void update() override;

  // ESPHome calls this to draw pixels
  void draw_pixel_at(int x, int y, esphome::Color color) override;

 protected:
  int get_width_internal() override { return 960; }   // M5Paper S3 resolution
  int get_height_internal() override { return 540; }

  void dump_config() override;

 private:
  void flush_();  // push framebuffer to epaper

  // ---- frame buffer (4-bit grayscale) ----
  std::vector<uint8_t> fb_;

  inline void draw_absolute_pixel_internal(int x, int y, uint8_t idx) {
    int p = y * this->get_width() + x;
    int byte_index = p >> 1;
    bool high = !(p & 1);

    uint8_t &b = fb_[byte_index];
    if (high)
      b = (b & 0x0F) | (idx << 4);
    else
      b = (b & 0xF0) | idx;
  }

  std::atomic<bool> dirty_{false};
  M5GFX *gfx_ = nullptr;
};
};
