#pragma once

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
  ~M5PaperS3DisplayM5GFX() override;

  // Required by Display base class
  void draw_pixel_at(int x, int y, esphome::Color color) override;

 protected:
  // Canvas (LGFX sprite) allocated in setup
  lgfx::v1::LGFX_Sprite *canvas_{nullptr};

  // State
  bool initialized_{false};  // true after setup succeeded
  std::atomic<bool> dirty_{false};  // set when draw_pixel_at changed pixels

  // Helpers
  uint8_t color_to_gray4(const esphome::Color &c);
  void ensure_canvas_created();
  void flush_canvas_to_display();  // pushes sprite to M5.Display and calls display()
};

}  // namespace m5papers3_display_m5gfx
}  // namespace esphome
