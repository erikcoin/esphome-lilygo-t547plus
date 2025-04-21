#include "my_display.h"

namespace esphome {
namespace my_display {

void MyEpaperDisplay::setup() {
  gfx.init();  // Initialiseer M5GFX
  gfx.setRotation(0);
  gfx.setEpdMode(epd_mode_t::epd_quality);  // evt. relevant voor je e-paper
  gfx.fillScreen(TFT_WHITE);                // Maak scherm wit bij start

  // Canvas/Sprite maken
  LGFX_Sprite canvas(&gfx);                 // Koppel canvas aan het display
  canvas.createSprite(250, 122);            // Pas formaat aan aan je scherm
  canvas.fillScreen(TFT_WHITE);             // Achtergrondkleur

  canvas.setTextColor(TFT_BLACK);
  canvas.setTextSize(2);
  canvas.drawString("Hallo ESPHome!", 10, 20);  // Teken tekst op canvas

  canvas.pushSprite(0, 0);                  // Teken canvas op scherm
  canvas.deleteSprite();                    // Opruimen
  ESP_LOGI("my_display", "Scherm wit gemaakt bij opstart.");
}

void MyEpaperDisplay::update() {
  // geen rendering via lambda nodig hier
}

}  // namespace my_display
}  // namespace esphome
