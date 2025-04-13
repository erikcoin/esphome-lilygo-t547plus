#include "m5papers3epd.h"

namespace esphome {
namespace m5papers3ns {

void M5PaperS3EPD::setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  display_.begin();
  display_.setRotation(0);
  display_.fillScreen(TFT_WHITE);
  display_.display();
}

void M5PaperS3EPD::update() {
  display_.fillScreen(TFT_WHITE);
  display_.setTextColor(TFT_BLACK);
  display_.setTextSize(size_);
  display_.drawString(text_.c_str(), x_, y_);
  display_.display();  // Push to EPD
}

}  // namespace m5papers3ns
}  // namespace esphome
