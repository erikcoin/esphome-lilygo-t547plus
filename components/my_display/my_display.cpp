#include "my_display.h"
#include "esphome/core/log.h"

namespace esphome {
namespace my_display {

static const char *const TAG = "my_display";

void MyDisplay::setup() {
  ESP_LOGI(TAG, "Setting up display...");

  display_.begin();
  display_.setEpdMode(epd_mode_t::epd_quality);
  display_.clearDisplay();
  display_.setTextColor(TFT_BLACK);
  display_.setTextSize(2);
  display_.setCursor(10, 10);
  display_.print("Init Display");
  display_.display();  // push to screen
}

void MyDisplay::update() {
  ESP_LOGI(TAG, "Updating display...");

  display_.clearDisplay();
  display_.setCursor(10, 10);
  display_.print("Updated!");
  display_.display();
}

void MyDisplay::draw_absolute_pixel_internal(int x, int y, Color color) {
  // Optional if you want display buffering via ESPHome API
  display_.drawPixel(x, y, color.raw_8);
}

void MyDisplay::dump_config() {
  ESP_LOGCONFIG(TAG, "MyDisplay configured.");
}

}  // namespace my_display
}  // namespace esphome
