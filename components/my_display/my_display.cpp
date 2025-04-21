#include "my_display.h"
#include "esphome/core/log.h"

namespace esphome {
namespace my_display22 {

static const char *const TAG = "my_display";

MyEpaperDisplay::MyEpaperDisplay() {}

void MyEpaperDisplay::setup() {
  bool ok = this->gfx_.begin();
  ESP_LOGD(TAG, "gfx.begin() resultaat: %s", ok ? "OK" : "MISLUKT");
  this->gfx_.setRotation(0);
  this->gfx_.fillScreen(TFT_WHITE);
  this->gfx_.display();
}

void MyEpaperDisplay::update() {
  // Geen verdere acties tijdens update
}

void MyEpaperDisplay::draw_absolute_pixel_internal(int x, int y, Color color) {
  // Geen implementatie nodig voor deze test
}

display::DisplayType MyEpaperDisplay::get_display_type() {
  return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
}

int MyEpaperDisplay::get_width_internal() {
  return 960;
}

int MyEpaperDisplay::get_height_internal() {
  return 540;
}

}  // namespace my_display22
}  // namespace esphome
