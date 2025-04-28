#include "esphome.h"
#include "M5Unified.h"

class M5TouchComponent : public Component, public Touchscreen {
 public:
  void setup() override {
    ESP_LOGD("m5touch", "Touchscreen setup done via M5Unified");
    // M5Unified already initialized the touchscreen
  }

  void update() override {
    M5.update();  // Always update M5 state
    auto t = M5.Touch.getDetail();
    if (t.isTouch()) {
      // Touch detected, report it to ESPHome
      this->touch(t.x, t.y);
    }
  }
};
