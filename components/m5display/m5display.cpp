#include "esphome.h"
#include <epdiy.h>
#include <M5Unified.h>

class M5PaperDisplay : public Component {
 public:
  void setup() override {
    // Initialize M5 Paper display
    m5.begin();
    m5.lcd.setTextColor(TFT_WHITE);
    m5.lcd.setTextSize(2);
    m5.lcd.setCursor(10, 10);
    m5.lcd.print("Hello, M5 Paper!");
  }

  void loop() override {
    // Your display update logic goes here
    // For example, refreshing text, updating screen, etc.
  }
};
