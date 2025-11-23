#pragma once

#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/core/log.h"
#include "esphome/components/display/display.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/automation.h" 

#include <M5Unified.h>
#include <M5GFX.h>
#include <lvgl.h>
#include <map>
#include <vector> 

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
    std::string buttonid;
    bool is_pressed = false;
    Trigger<> *trigger = nullptr;

};

class M5PaperS3DisplayM5GFX : public display::Display {
 public:
    void setup() override;
    void loop();
    void dump_config() override;
    void partial_update(int x, int y, int w, int h);
    float get_setup_priority() const override { return setup_priority::HARDWARE; }
    void update() override;
    ~M5PaperS3DisplayM5GFX();
Trigger<> *make_button_trigger(const std::string &buttonid);


   //Trigger<> *make_button_trigger() { return &this->release_trigger_; }
    void add_button(int x, int y, int width, int height, const std::string &buttonid, Trigger<> *trigger);
    void draw_button(int index);
    void press_button_effect(int index, int duration_ms = 150);

    void fill(Color color) override;
    int get_width_internal() override;
    int get_height_internal() override;
    void draw_pixel_at(int x, int y, esphome::Color color) override;
    void draw_all_buttons() ;
    display::DisplayType get_display_type() override {
        return display::DisplayType::DISPLAY_TYPE_GRAYSCALE;
    }

    void set_rotation(int rotation);
    void set_writer(std::function<void(display::Display &)> writer);
    void set_touch_sensor(text_sensor::TextSensor *sensor);


 protected:
    void update_touch();
    bool get_touch(TouchPoint *point);
    void send_coordinates_and_check_buttons(TouchPoint tp); // Modified to check buttons

    int rotation_{0};
    lgfx::v1::LGFX_Sprite *canvas_{nullptr};
lgfx::v1::LGFX_Sprite* button1Sprite;
lgfx::v1::LGFX_Sprite* button2Sprite;
    std::function<void(display::Display &)> writer_{nullptr};
    m5gfx::LGFX_Device& gfx_ = M5.Display;
    text_sensor::TextSensor *touch_coordinates_sensor_{nullptr};
  //  Trigger<> release_trigger_;
 //   std::vector<std::unique_ptr<Trigger<>>> triggers_;  // Bewaart alle unieke triggers
//std::map<std::string, std::unique_ptr<Trigger<>>> button_triggers_;
//std::map<std::string, bool> button_states_;
    std::vector<ButtonConfig> buttons_{}; // Vector to store configured buttons

};

} // namespace m5papers3_display_m5gfx
} // namespace esphome
