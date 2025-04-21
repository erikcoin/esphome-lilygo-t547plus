#include "m5papers3_display.h" // Zorg dat de .h bestandsnaam klopt
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

// --- Component Overrides ---
void M5PaperS3DisplayM5GFX::setup() {
  ESP_LOGCONFIG(TAG, "Setting up M5Paper S3 display using M5GFX...");
  M5.begin();
  ESP_LOGD(TAG, "M5.begin() finished.");

  auto &gfx = M5.Display;
  gfx.setRotation(this->rotation_);
  ESP_LOGD(TAG, "M5GFX Rotation set to: %d", this->rotation_);

  // !! Gebruik lgfx::LGFX_Sprite bij het initialiseren van de canvas !!
  this->canvas_ = lgfx::LGFX_Sprite(&gfx); // Correcte type

  int w = gfx.width();
  int h = gfx.height();
  this->canvas_.createSprite(w, h); // Nu zou canvas_ moeten bestaan
  ESP_LOGD(TAG, "M5GFX Sprite created with size: %d x %d", w, h);

  this->canvas_.setColorDepth(8);
  ESP_LOGD(TAG, "Sprite color depth set to %d", this->canvas_.getColorDepth());

  // !! Gebruik Color::WHITE ipv COLOR_WHITE !!
  uint32_t white_color = get_native_m5gfx_color_(Color::WHITE);
  gfx.fillScreen(white_color);
  this->canvas_.fillSprite(white_color); // Nu zou canvas_ moeten bestaan

  ESP_LOGCONFIG(TAG, "M5Paper S3 M5GFX Display setup complete.");
}

void M5PaperS3DisplayM5GFX::dump_config() {
  LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper", this);
  ESP_LOGCONFIG(TAG, "  Rotation: %d degrees", this->rotation_ * 90);
  // !! LOG_UPDATE_INTERVAL zou nu moeten werken omdat ambiguïteit is opgelost !!
  LOG_UPDATE_INTERVAL(this);
}

// --- PollingComponent / Display Override ---
void M5PaperS3DisplayM5GFX::update() {
  ESP_LOGD(TAG, "Running M5GFX display update...");
  if (this->writer_ != nullptr) {
    this->writer_(*this);
  }

  ESP_LOGD(TAG, "Pushing M5GFX sprite to display...");
  this->canvas_.pushSprite(0, 0); // Nu zou canvas_ moeten bestaan

  ESP_LOGD(TAG, "M5GFX display update finished (EPD refresh status unknown).");
}

// --- Display Overrides ---
void M5PaperS3DisplayM5GFX::set_rotation(int rotation) {
  // Implementatie blijft hetzelfde
  int m5gfx_rotation = 0;
  switch (rotation) {
    case 90: m5gfx_rotation = 1; break;
    case 180: m5gfx_rotation = 2; break;
    case 270: m5gfx_rotation = 3; break;
    default: m5gfx_rotation = 0; break;
  }
  this->rotation_ = m5gfx_rotation;
}

int M5PaperS3DisplayM5GFX::get_width_internal() {
  // Implementatie blijft hetzelfde
  if (M5.getBoard() == m5::board_t::board_unknown && !this->is_setup_) return 960;
  return M5.Display.width();
}

int M5PaperS3DisplayM5GFX::get_height_internal() {
  // Implementatie blijft hetzelfde
   if (M5.getBoard() == m5::board_t::board_unknown && !this->is_setup_) return 540;
  return M5.Display.height();
}

void M5PaperS3DisplayM5GFX::fill(Color color) {
  // Implementatie blijft hetzelfde, maar canvas_ zou nu moeten werken
  uint32_t native_color = get_native_m5gfx_color_(color);
  this->canvas_.fillSprite(native_color);
}

// --- Protected Display Overrides ---
void M5PaperS3DisplayM5GFX::draw_absolute_pixel_internal(int x, int y, Color color) {
  // Implementatie blijft hetzelfde, maar canvas_ zou nu moeten werken
   uint32_t native_color = get_native_m5gfx_color_(color);
   this->canvas_.drawPixel(x, y, native_color);
}

// --- Helper Functie ---
uint32_t M5PaperS3DisplayM5GFX::get_native_m5gfx_color_(Color color) {
    // !! Gebruik R, G, B componenten en luminantie voor grayscale !!
    float r = color.get_red();     // 0.0 - 1.0
    float g = color.get_green();   // 0.0 - 1.0
    float b = color.get_blue();    // 0.0 - 1.0
    // Luminantie formule (gewogen gemiddelde)
    float gray_f = (r * 0.2126f + g * 0.7152f + b * 0.0722f);
    // Schaal naar 0-255
    uint8_t gray_8bit = static_cast<uint8_t>(gray_f * 255.0f);

    // Converteer 8-bit gray naar M5GFX kleur (RGB888 formaat)
    return M5.Display.color888(gray_8bit, gray_8bit, gray_8bit);
}

} // namespace m5papers3_display_m5gfx
} // namespace esphome
