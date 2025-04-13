#pragma once

#include "esphome.h"
#include <epdiy.h>
#include <M5Unified.h>  // Include the M5Unified library

class M5PaperDisplay : public PollingComponent {  // Inherit from PollingComponent
 public:
  M5PaperDisplay() : PollingComponent(1000) {}  // Update interval: 1s

  void setup() override {
    // Initialize M5 Paper display
    m5.begin();
    m5.lcd.setTextColor(TFT_WHITE);
    m5.lcd.setTextSize(2);
    m5.lcd.setCursor(10, 10);
    m5.lcd.print("Hello, M5 Paper!");
  }

  void update() override {
    // This will be called every 1s as defined in PollingComponent constructor
    m5.lcd.setCursor(10, 40);
    m5.lcd.print("Updated text");
  }
};
