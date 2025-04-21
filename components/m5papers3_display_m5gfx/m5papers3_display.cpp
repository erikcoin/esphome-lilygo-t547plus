#include "m5papers3_display.h" // Let op: H-file naam moet matchen
#include "esphome/core/log.h"
#include "esphome/core/application.h"

// M5Unified/M5GFX zijn al in de .h geinclude

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

void M5PaperS3DisplayM5GFX::setup() {
  ESP_LOGCONFIG(TAG, "Setting up M5Paper S3 display using M5GFX...");

  // Initialiseer M5 board. M5.begin() zou M5.Display (M5GFX instance)
  // moeten configureren voor het EPD op de S3.
  M5.begin();
  ESP_LOGD(TAG, "M5.begin() finished.");

  // M5.Display is nu onze M5GFX driver instance voor het EPD
  auto &gfx = M5.Display; // Gebruik referentie voor kortere code

  // Stel rotatie in via M5GFX
  gfx.setRotation(this->rotation_); // M5GFX gebruikt 0, 1, 2, 3 voor 0, 90, 180, 270
  ESP_LOGD(TAG, "M5GFX Rotation set to: %d", this->rotation_);

  // Maak de M5GFX sprite (canvas) aan
  // Koppel aan de M5.Display driver
  this->canvas_ = M5GFX::LGFX_Sprite(&gfx);
  // Maak sprite met de afmetingen van het display (na rotatie)
  // Let op: M5GFX width()/height() houden al rekening met rotatie
  int w = gfx.width();
  int h = gfx.height();
  this->canvas_.createSprite(w, h);
  ESP_LOGD(TAG, "M5GFX Sprite created with size: %d x %d", w, h);

  // Optioneel: Stel kleurdiepte van sprite in (als EPD > 1bpp is)
  // Voor grayscale EPD is 4bpp (16 kleuren) of 8bpp (256 kleuren) logisch.
  // Laten we 8bpp proberen voor meer grijstinten. Standaard is vaak 16bpp (RGB565).
  this->canvas_.setColorDepth(8); // 8 bits per pixel (grayscale)
  ESP_LOGD(TAG, "Sprite color depth set to %d", this->canvas_.getColorDepth());

  // Wis het scherm initieel (gebruik M5GFX kleurconstante of helper)
  uint32_t white_color = get_native_m5gfx_color_(COLOR_WHITE);
  gfx.fillScreen(white_color); // Wis fysiek scherm (kan langzaam zijn!)
  this->canvas_.fillSprite(white_color); // Wis sprite buffer

  ESP_LOGCONFIG(TAG, "M5Paper S3 M5GFX Display setup complete.");
}

void M5PaperS3DisplayM5GFX::dump_config() {
  LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper", this);
  ESP_LOGCONFIG(TAG, "  Rotation: %d degrees", this->rotation_ * 90); // *90 voor weergave
  LOG_UPDATE_INTERVAL(this);
}

void M5PaperS3DisplayM5GFX::update() {
  ESP_LOGD(TAG, "Running M5GFX display update...");
  // 1. Roep lambda aan om op de canvas te tekenen
  if (this->writer_ != nullptr) {
    this->writer_(*this);
  }

  // 2. Push de sprite naar het display
  ESP_LOGD(TAG, "Pushing M5GFX sprite to display...");
  this->canvas_.pushSprite(0, 0);

  // 3. Trigger EPD refresh (!!! ONZEKER PUNT !!!)
  // M5GFX heeft geen standaard .display() of EPD update.
  // Mogelijkheid 1: pushSprite triggert het al via de panel driver (ingesteld door M5.begin).
  // Mogelijkheid 2: M5Unified voegt een methode toe aan M5.Display specifiek voor EPD?
  // Mogelijkheid 3: We moeten wachten tot het display niet meer bezig is?
  //    while (M5.Display.displayBusy()) { // Check of deze methode bestaat/werkt voor EPD
  //      yield(); // Geef tijd aan andere processen
  //    }
  // Mogelijkheid 4: Er is geen directe manier en het werkt niet goed zonder M5EPD.

  // We proberen zonder expliciete trigger en hopen op Mogelijkheid 1.
  // Als het scherm niet update, is hier het probleem.
  ESP_LOGD(TAG, "M5GFX display update finished (EPD refresh status unknown).");
}

void M5PaperS3DisplayM5GFX::set_rotation(int rotation) {
  // ESPHome rotation (0, 90, 180, 270) naar M5GFX rotation (0, 1, 2, 3)
  int m5gfx_rotation = 0;
  switch (rotation) {
    case 90: m5gfx_rotation = 1; break;
    case 180: m5gfx_rotation = 2; break;
    case 270: m5gfx_rotation = 3; break;
    default: m5gfx_rotation = 0; break;
  }
  this->rotation_ = m5gfx_rotation;
  // Pas direct toe als al gesetup? Kan width/height beïnvloeden.
  // if (this->is_setup_) { M5.Display.setRotation(this->rotation_); }
}

// Retourneert breedte zoals M5GFX die rapporteert (houdt rekening met rotatie)
int M5PaperS3DisplayM5GFX::get_width_internal() {
  // Check of M5.Display al geïnitialiseerd is
  if (M5.getBoard() == m5::board_t::board_unknown) return 960; // Default S3 width
  return M5.Display.width();
}

// Retourneert hoogte zoals M5GFX die rapporteert (houdt rekening met rotatie)
int M5PaperS3DisplayM5GFX::get_height_internal() {
  if (M5.getBoard() == m5::board_t::board_unknown) return 540; // Default S3 height
  return M5.Display.height();
}

void M5PaperS3DisplayM5GFX::fill(Color color) {
  uint32_t native_color = get_native_m5gfx_color_(color);
  // Vul de canvas/sprite
  this->canvas_.fillSprite(native_color);
}

void M5PaperS3DisplayM5GFX::draw_absolute_pixel_internal(int x, int y, Color color) {
   uint32_t native_color = get_native_m5gfx_color_(color);
   // Teken op de canvas/sprite
   this->canvas_.drawPixel(x, y, native_color);
}

// Helper: Converteer ESPHome Color naar M5GFX kleur (uint32_t, bv. 0xRRGGBB)
// We mikken op grayscale omdat het een EPD is.
uint32_t M5PaperS3DisplayM5GFX::get_native_m5gfx_color_(Color color) {
    // Converteer naar 8-bit grayscale (0=zwart, 255=wit)
    float gray_f = color.to_grayscale(); // 0.0 tot 1.0
    uint8_t gray_8bit = static_cast<uint8_t>(gray_f * 255.0f);

    // Maak een M5GFX kleur aan met gelijke R, G, B waarden
    // M5.Display.colorXXX functies zijn handig, of direct RRGGBB maken.
    // Gebruik color888 die een 24-bit kleur (uint32_t) retourneert.
    return M5.Display.color888(gray_8bit, gray_8bit, gray_8bit);

    /* Alternatief: M5GFX TFT_BLACK / TFT_WHITE
    if (gray_8bit < 128) { // Dichter bij zwart
        return TFT_BLACK; // Is vaak gedefinieerd als 0x0000
    } else { // Dichter bij wit
        return TFT_WHITE; // Is vaak gedefinieerd als 0xFFFF of 0xFFFFFF
                          // Gebruik specifieke M5GFX waarde indien nodig.
    }
    */
    // Belangrijk: De M5GFX driver voor het IT8951 panel moet deze kleur
    // intern correct omzetten naar de native 4-bit grayscale van het EPD.
}

} // namespace m5papers3_display_m5gfx
} // namespace esphome
