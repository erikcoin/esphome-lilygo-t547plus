#include "m5papers3_epd.h"

namespace esphome {
namespace m5papers3ns {

void M5PaperS3EPD::setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  display_.begin();
  display_.setRotation(0);
  display_.fillScreen(TFT_WHITE);
  draw_demo();
}

void M5PaperS3EPD::loop() {
  // Optional: Update logic
}

void M5PaperS3EPD::draw_demo() {
  display_.setTextColor(TFT_BLACK);
  display_.setTextSize(2);
  display_.drawString("Hello from M5PaperS3!", 10, 10);
}

}  // namespace m5papers3ns
}  // namespace esphome
