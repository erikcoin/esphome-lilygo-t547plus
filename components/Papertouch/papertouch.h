#pragma once

#include "esphome/components/touchscreen/touchscreen.h"
#include "esphome/components/display/display.h"

#include <M5GFX.h>  // Zorg dat M5GFX beschikbaar is!
#include <M5Unified.h>
namespace esphome {
namespace papertouch {

class Papertouch : public touchscreen::TouchscreenComponent {
 public:
  void setup() override;
  void update_touches() override;

  void set_display(display::Display* display) { this->display_ = display; }

 protected:
  display::Display* display_;
};

}  // namespace papertouch
}  // namespace esphome
