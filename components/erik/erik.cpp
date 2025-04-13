#include "erik.h"

namespace esphome {
namespace erik {

void ErikComponent::setup() {
  auto cfg = M5.config();  // Optional, but allows customization
  M5.begin(cfg);           // Init M5 stack

  M5.Display.setRotation(0);
  M5.Display.fillScreen(TFT_WHITE);
  M5.Display.display();  // Commit the clear screen to EPD
}

void ErikComponent::update() {
  M5.Display.fillScreen(TFT_WHITE);
  M5.Display.setTextColor(TFT_BLACK);
  M5.Display.setTextSize(size_);
  M5.Display.drawString(text_.c_str(), x_, y_);
  M5.Display.display();  // Push to EPD
}

}  // namespace erik
}  // namespace esphome
