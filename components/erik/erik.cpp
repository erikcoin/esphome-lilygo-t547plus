#include "erik.h"
#include "esphome/core/log.h"

namespace esphome {
namespace erik {

void ErikDisplay::setup() {
  M5.begin();
  M5.Lcd.setRotation(3);  // Adjust rotation as needed
}

void ErikDisplay::update() {
  // Update touch screen state
  M5.Touch.update(millis());

  // Perform other update actions, like checking for touch events
  if (M5.Touch.isPressed()) {
    // Handle touch events here (e.g., toggle light)
  }
}

void ErikDisplay::draw_display() {
  // Clear the screen
  M5.Lcd.fillScreen(TFT_BLACK);

  // Draw text or other graphics
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(10, 10);
  M5.Lcd.print("Bibliotheek Lamp");

  // Draw button (example)
  draw_button_(true);  // Change 'true' to the actual light state
}

void ErikDisplay::draw_button_(bool state) {
  if (state) {
    M5.Lcd.fillRect(50, 200, 200, 50, TFT_GREEN);
    M5.Lcd.setTextColor(TFT_BLACK);
    M5.Lcd.setCursor(70, 215);
    M5.Lcd.print("Turn Off");
  } else {
    M5.Lcd.fillRect(50, 200, 200, 50, TFT_RED);
    M5.Lcd.setTextColor(TFT_BLACK);
    M5.Lcd.setCursor(70, 215);
    M5.Lcd.print("Turn On");
  }
}

void ErikDisplay::toggle_light_() {
  // Toggle light state (for example, you could make an API call to Home Assistant)
  // Example of calling a service:
  auto call = esphome::api::global_api_server->make_service_call();
  // Toggle logic goes here
}

}  // namespace erik
}  // namespace esphome
