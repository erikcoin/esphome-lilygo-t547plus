#include "my_display.h"
#include <epdiy.h>
#include <M5GFX.h> // M5GFX voor display functies

MyEpaperDisplay::MyEpaperDisplay() //: DisplayBuffer(960, 540) {} // Resolutie van het ED047TC1 scherm

void MyEpaperDisplay::setup() {
  // Initieer de M5GFX bibliotheek
  gfx.begin();
  gfx.setRotation(1); // Zet de rotatie van het scherm (pas aan naar behoefte)
  gfx.setTextColor(TFT_BLACK); // Zet de tekstkleur
  gfx.setTextSize(1); // Zet de tekstgrootte
  gfx.fillScreen(TFT_WHITE); // Vul het scherm met wit
  gfx.display(); // Werk de weergave bij
}

void MyEpaperDisplay::update() {
  this->do_draw_(); // Teken de pixeldata naar het scherm
  gfx.display(); // Werk het scherm bij
}

void MyEpaperDisplay::draw_absolute_pixel_internal(int x, int y, esphome::Color color) {
  // Teken individuele pixels, afhankelijk van de kleur
  gfx.drawPixel(x, y, color.is_on() ? TFT_BLACK : TFT_WHITE);
}

void MyEpaperDisplay::fill(esphome::Color color) {
  // Vul het scherm met de opgegeven kleur
  gfx.fillScreen(color.is_on() ? TFT_BLACK : TFT_WHITE);
}
