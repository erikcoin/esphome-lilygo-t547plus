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
  std::atomic_bool lvgl_busy_{false};   // small reentrancy guard
  uint32_t lvgl_last_tick_ms_{0};       // throttling timer
 public:
    void setup() override;
    void loop();
  //  void dump_config() override;
 //   void partial_update(int x, int y, int w, int h);
    float get_setup_priority() const override { return setup_priority::HARDWARE; }
    void update() override;
    ~M5PaperS3DisplayM5GFX();
  //  void fill(Color color) override;
    int get_width_internal() override;
    int get_height_internal() override;
    void draw_pixel_at(int x, int y, esphome::Color color) override;
    display::DisplayType get_display_type() override {
        return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
    }

    void set_rotation(int rotation);
    void set_writer(std::function<void(display::Display &)> writer);
    static void lvgl_flush_impl(disp, area, color_p);
    static void lvgl_flush_wrapper(lv_disp_drv_t *disp,
                                   const lv_area_t *area,
                                   lv_color_t *color_p);

// LVGL-related:
void lvgl_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p);
 // void lvgl_flush(const lv_area_t *area, lv_color_t *color_p);
  // override to run lv handler
 // void update() override;

 protected:
// Worker job data (protected)
lv_area_t pending_area_;          // area to flush (copied by value)
lv_color_t *pending_buf_{nullptr}; // pointer to LVGL buffer for the area (owned by producer)
SemaphoreHandle_t flush_sem_{nullptr}; // semaphore that signals worker
  static void flush_worker_task_trampoline(void *arg);
  void flush_worker_task();
// Helper to draw an area (called by worker)
void draw_fast_area(const lv_area_t &area, lv_color_t *color_p);
  static void lvgl_task_trampoline(void *arg);

  void lvgl_task();              // actual member that runs lv_timer_handler()
  TaskHandle_t lvgl_task_handle_{nullptr};
  // in M5PaperS3DisplayM5GFX class
lv_disp_draw_buf_t draw_buf_;
lv_color_t *lv_buf1_{nullptr};
lv_color_t *lv_buf2_{nullptr};
lv_disp_drv_t disp_drv_;
void lvgl_flush_cb(const lv_area_t *area, lv_color_t *color_p);
// PSRAM line buffers (allocated in setup)
uint16_t *linebufA_{nullptr};
uint16_t *linebufB_{nullptr};
size_t linebuf_capacity_{0}; // number of pixels each line buffer can hold
    // Static trampoline (LVGL calls this)
    static void lvgl_flush_cb_trampoline(
        lv_disp_drv_t *drv,
        const lv_area_t *area,
        lv_color_t *color_p);

  // size of LVGL buffer in lines:
  static constexpr int LV_BUF_LINES = 40;   // tweakable
    int rotation_{0};
    //lgfx::v1::LGFX_Sprite *canvas_{nullptr};
    std::function<void(display::Display &)> writer_{nullptr};
    m5gfx::LGFX_Device& gfx_ = M5.Display;
};

} // namespace m5papers3_display_m5gfx
} // namespace esphome
