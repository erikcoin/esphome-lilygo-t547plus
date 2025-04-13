#pragma once

#include "esphome.h"
#include <epdiy.h>
#include <M5Unified.h>

class M5PaperDisplay : public PollingComponent {
 public:
  M5PaperDisplay() : PollingComponent(1000) {}  // Set to update every 1 second

  void setup() override {
    // Initialize M5 Paper
    m5.begin();
    m5.lcd.setTextColor(TFT_WHITE);
    m5.lcd.setTextSize(2);
    m5.lcd.setCursor(10, 10);
    m5.lcd.print("Hello, M5 Paper!");
  }

  void update() override {
    // This will be called every 1 second (because of PollingComponent)
    m5.lcd.setCursor(10, 40);
    m5.lcd.print("Updated text");
  }
};
