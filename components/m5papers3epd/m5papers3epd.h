
#pragma once

#include "esphome/core/component.h"
#include <epdiy.h>
#include <M5Unified.h>


namespace esphome {
namespace m5papers3ns {

class M5PaperS3EPD : public Component {
 public:
  void setup() override;
  void loop() override;

 protected:
  M5GFX display_;
  void draw_demo();
};

}  // namespace m5papers3ns
}  // namespace esphome
