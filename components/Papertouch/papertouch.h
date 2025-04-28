#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome.h"
#include "esphome/core/log.h"
#include "M5Unified.h"

using namespace esphome;

namespace papertouch {

class Papertouch : public Component {
public:
    Papertouch() : display(nullptr) {}

    void setup() override {
        // Ensure the touch screen is initialized
        M5.begin();  // Initialize the M5 device (including touch)
        M5.Touch.begin(&M5.Lcd);  // Initialize touch with the screen device

        // Additional setup code here
    }

    void update() override {
        if (M5.Touch.isPressed()) {
            // Get the touch coordinates
            TouchPoint touch = M5.Touch.getPressPoint();
//            ESP_LOGD("papertouch", "Touch detected at: %d, %d", touch.x, touch.y);
            
            // You can implement logic for touch events here
        }
    }

    void set_display(M5GFX* display) {
        this->display = display;
    }

    void set_update_interval(uint32_t interval) {
        this->update_interval = interval;
    }

    void set_component_source(const std::string& source) {
        this->component_source = source;
    }

private:
    M5GFX* display;
    uint32_t update_interval;
    std::string component_source;
};

}  // namespace papertouch
