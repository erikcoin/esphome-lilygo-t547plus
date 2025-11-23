#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/display/display.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/automation.h" 

#include <M5Unified.h>
#include <M5GFX.h>
#include <lvgl.h>
#include <map>
#include <vector> 

namespace esphome {
namespace text_sensor { class TextSensor; }
}

namespace lgfx { namespace v1 { class LGFX_Sprite; } }
namespace esphome {
namespace m5papers3_display_m5gfx {

class M5PaperS3DisplayM5GFX : public display::Display {
private:
  uint8_t *epd_buffer_{nullptr};  // 4-bit per pixel framebuffer
 public:
    void setup() override;
    void loop();
    void dump_config() override;
    void partial_update(int x, int y, int w, int h);
    float get_setup_priority() const override { return setup_priority::HARDWARE; }
    void update() override;
    ~M5PaperS3DisplayM5GFX();
  //  void fill(Color color) override;
  //  int get_width_internal() override;
  //  int get_height_internal() override;
   // void draw_pixel_at(int x, int y, esphome::Color color) override;
    display::DisplayType get_display_type() override {
        return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
    }

    void set_rotation(int rotation);
    void set_writer(std::function<void(display::Display &)> writer);

// LVGL-related:
  void lvgl_flush(const lv_area_t *area, lv_color_t *color_p);
  // override to run lv handler
 // void update() override;

 protected:

  // LVGL draw buffer
  lv_disp_draw_buf_t draw_buf_;
  lv_color_t *lv_buf1_;
  lv_color_t *lv_buf2_;
  lv_disp_drv_t disp_drv_;

  // size of LVGL buffer in lines:
  static constexpr int LV_BUF_LINES = 40;   // tweakable
    int rotation_{0};
    //lgfx::v1::LGFX_Sprite *canvas_{nullptr};
    std::function<void(display::Display &)> writer_{nullptr};
    m5gfx::LGFX_Device& gfx_ = M5.Display;
};

} // namespace m5papers3_display_m5gfx
} // namespace esphome
