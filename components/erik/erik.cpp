#include "erik.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace erik {

void ErikDisplay::setup() {
  M5.begin();
  M5.Display.setRotation(0);
  M5.Display.fillScreen(TFT_WHITE);
  M5.Display.display();
}

void ErikDisplay::update() {
  if (light_state_ == nullptr) return;

  bool is_on = light_state_->state == "on";

  M5.Display.fillScreen(TFT_WHITE);
  draw_button_(is_on);
  M5.Display.display();

  M5.Touch.update();
  if (M5.Touch.getCount() > 0) {
    auto t = M5.Touch.getDetail();
    if (t.x > 40 && t.x < 240 && t.y > 100 && t.y < 160) {
      toggle_light_();
    }
  }
}

void ErikDisplay::draw_button_(bool state) {
  int btn_x = 40;
  int btn_y = 100;
  int btn_w = 200;
  int btn_h = 60;

  uint32_t color = state ? TFT_GREEN : TFT_RED;
  const char *label = state ? "Lamp Aan" : "Lamp Uit";

  M5.Display.fillRoundRect(btn_x, btn_y, btn_w, btn_h, 10, color);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextDatum(middle_center);
  M5.Display.drawString(label, btn_x + btn_w / 2, btn_y + btn_h / 2);
}

void ErikDisplay::toggle_light_() {
  // Send a Home Assistant service call to toggle the light
  auto call = esphome::api::global_api_server->make_service_call();
  call.service = "toggle";
  call.service_data["entity_id"] = "light.bibliotheeklamp";
  call.service_domain = "light";
  call.perform();
}

}  // namespace erik
}  // namespace esphome
