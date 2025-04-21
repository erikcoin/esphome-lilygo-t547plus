
#include "my_display.h"

namespace esphome {
namespace my_display {

void MyDisplay::setup() {
  gfx_.begin();
  canvas_.setColorDepth(1);
  canvas_.createSprite(gfx_.width(), gfx_.height());
  canvas_.fillScreen(TFT_WHITE);
  canvas_.pushSprite(0, 0);
}

void MyDisplay::update() {
  canvas_.fillScreen(TFT_WHITE);
  canvas_.fillTriangle(60, 60, 120, 180, 180, 60, TFT_BLACK);
  canvas_.pushSprite(0, 0);
}

void MyDisplay::draw_absolute_pixel_internal(int x, int y, int color) {
  canvas_.drawPixel(x, y, color);
}

}  // namespace my_display
}  // namespace esphome
