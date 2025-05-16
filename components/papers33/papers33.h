#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/display/display.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/automation.h" // <-- Include for Automation

#include <M5Unified.h>
#include <M5GFX.h>

#include <vector> // <-- Include for std::vector

namespace esphome {
namespace text_sensor { class TextSensor; }
}

namespace lgfx { namespace v1 { class LGFX_Sprite; } }


namespace esphome {
namespace m5papers3_display_m5gfx {

struct TouchPoint {
    uint16_t x;
    uint16_t y;
};

// Structure to hold button data
struct ButtonConfig {
    int x;
    int y;
    int width;
    int height;
    Automation<> *on_press_automation; // Pointer to the automation to trigger
    // Automation<> *on_release_automation; // For future
};

class M5PaperS3DisplayM5GFX : public display::Display {
 public:
    void setup() override;
    void dump_config() override;
    void partial_update(int x, int y, int w, int h);
    float get_setup_priority() const override { return setup_priority::HARDWARE; }
    void update() override;
    ~M5PaperS3DisplayM5GFX();
    //Trigger<> *get_on_press_trigger(int button_index);
    void fill(Color color) override;
    int get_width_internal() override;
    int get_height_internal() override;
    void draw_pixel_at(int x, int y, esphome::Color color) override;

    display::DisplayType get_display_type() override {
        return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
    }

    void set_rotation(int rotation);
    void set_writer(std::function<void(display::Display &)> writer);
    void set_touch_sensor(text_sensor::TextSensor *sensor);

    // Method to add a button from generated code
    void add_button(int x, int y, int width, int height, Automation<> *on_press_automation);

 protected:
    void update_touch();
    bool get_touch(TouchPoint *point);
    void send_coordinates_and_check_buttons(TouchPoint tp); // Modified to check buttons

    int rotation_{0};
    lgfx::v1::LGFX_Sprite *canvas_{nullptr};
    std::function<void(display::Display &)> writer_{nullptr};
    m5gfx::LGFX_Device& gfx_ = M5.Display;
    text_sensor::TextSensor *touch_coordinates_sensor_{nullptr};

    std::vector<ButtonConfig> buttons_{}; // Vector to store configured buttons
    //std::map<int, Trigger<>> on_press_triggers_{};
};

} // namespace m5papers3_display_m5gfx
} // namespace esphome
