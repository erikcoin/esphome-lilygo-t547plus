#pragma once

#include "esphome.h"
#include <M5Unified.h>
#include "lvgl.h"

namespace esphome {
namespace m5papers3_lvgl {

class M5PaperS3LVGL : public Component {
 public:
  M5PaperS3LVGL(uint16_t width = 960, uint16_t height = 540, uint16_t buf_lines = 40);

  void setup() override;
  void loop() override;
  void dump_config() override;

  // Called from LVGL via static wrapper:
  void lvgl_flush(const lv_area_t *area, lv_color_t *color_p);

 protected:
  uint16_t width_;
  uint16_t height_;
  uint16_t buf_lines_;  // how many lines the lv buffer holds (tune to memory)

  lv_disp_draw_buf_t draw_buf_;
  lv_color_t *buf1_;    // allocated dynamically

  lv_disp_drv_t disp_drv_;

  // helper
  inline uint8_t rgb565_to_gray4(uint16_t color565);
};

}  // namespace m5papers3_lvgl
}  // namespace esphome
