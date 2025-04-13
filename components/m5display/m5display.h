#pragma once

#include "esphome.h"
#include <epdiy.h>
#include <M5Unified.h>

class M5PaperDisplay : public Component {
 public:
  void setup() override;
  void loop() override;
};
