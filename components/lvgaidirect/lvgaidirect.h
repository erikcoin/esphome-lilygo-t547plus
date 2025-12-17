#pragma once

#include <esphome/components/display/display.h>

#include "esphome.h"
#include <M5Unified.h>
#include <M5GFX.h>


// LovyanGFX forward (we include proper header in cpp)
namespace lgfx {
namespace v1 {
class LGFX_Sprite;
}
}

namespace esphome {
namespace m5papers3_display_m5gfx {

class M5PaperS3DisplayM5GFX : public esphome::display::Display {
 public:
  void setup() override;
  void update() override;
  ~M5PaperS3DisplayM5GFX(); //override;
void poll_touch();
void loop();
  uint32_t sleep_duration_ms{0};
  uint32_t touch_gpio{48};
  //gpio_num_t touch_gpio{GPIO_NUM_NC};
  bool enable_touch_wakeup{false};
int last_touch_x_{0};
int last_touch_y_{0};
bool last_touch_pressed_{false};
  // Required by Display base class
  void draw_pixel_at(int x, int y, esphome::Color color) override;
    // NEW REQUIRED OVERRIDES
  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_BINARY; }
  int get_width_internal() override { return 540; }   // your panel width
  int get_height_internal() override { return 960; }  // your panel height
 
uint8_t* framebuffer_ = nullptr;   // 4-bit/pixel buffer in PSRAM
int fb_width_ = 0;
int fb_height_ = 0;

void draw_pixel_internal_at(int x, int y, uint8_t idx);
void flush_framebuffer_to_display();
//uint32_t sleep_duration_ms_{0};
//gpio_num_t touch_gpio_{GPIO_NUM_NC};
//bool enable_touch_wakeup_{false};
int64_t last_activity_{0};
  void set_sleep_duration(uint32_t ms) { sleep_duration_ms = ms; };
  void set_touch_gpio(int gpio) { touch_gpio = gpio; };
//void set_wakeup_pin(int pin) { wakeup_pin_ = pin; }
  void set_enable_touch_wakeup(bool en) { enable_touch_wakeup = en; };
protected:
  // Canvas (LGFX sprite) allocated in setup
//  lgfx::v1::LGFX_Sprite *canvas_{nullptr};

  // State
  bool initialized_{false};  // true after setup succeeded
  std::atomic<bool> dirty_{false};  // set when draw_pixel_at changed pixels

  // Helpers
  uint8_t color_to_gray4(const esphome::Color &c);
//  void ensure_canvas_created();
//  void flush_canvas_to_display();  // pushes sprite to M5.Display and calls display()
};

}  // namespace m5papers3_display_m5gfx
}  // namespace esphome
