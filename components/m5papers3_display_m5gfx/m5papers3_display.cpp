#include "m5papers3_display.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

void M5PaperS3DisplayM5GFX::setup() {
  ESP_LOGD(TAG, "Setting up M5Paper S3 display using M5GFX...");
  auto cfg = M5.config();
  M5.begin(cfg);
  ESP_LOGD(TAG, "M5.begin() finished.");

  M5.Display.setEpdMode(epd_mode_t::epd_fastest);
  while (!M5.Display.isReadable()) {
    ESP_LOGD(TAG, "Waiting for EPD to be ready...");
    delay(100);
  }

  M5.Display.clearDisplay();
  M5.Display.setRotation(this->rotation_);
  ESP_LOGD(TAG, "M5GFX Rotation set to: %d", this->rotation_);

  M5.Display.fillScreen(TFT_WHITE);
  M5.Display.display();
  M5.Display.waitDisplay();

  this->canvas_ = lgfx::LGFX_Sprite(&M5.Display);
  this->canvas_.setColorDepth(4);  // ✅ 4-bit = 16 grijsniveaus
  this->canvas_.createSprite(M5.Display.width(), M5.Display.height());
  ESP_LOGD(TAG, "M5GFX Sprite created with size: %d x %d", M5.Display.width(), M5.Display.height());

  this->canvas_.fillSprite(0);  // vul initieel met wit (index 0)

  ESP_LOGCONFIG(TAG, "M5Paper S3 M5GFX Display setup complete.");
}

void M5PaperS3DisplayM5GFX::dump_config() {
  LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper", this);
  ESP_LOGCONFIG(TAG, "  Rotation: %d degrees", this->rotation_ * 90);
  LOG_UPDATE_INTERVAL(this);
}

void M5PaperS3DisplayM5GFX::update() {
  static bool first_time = true;
  if (first_time) {
    ESP_LOGD(TAG, "Delaying first update for EPD...");
    delay(1000);
    first_time = false;
  }

  ESP_LOGD(TAG, "Running M5GFX display update...");

  this->canvas_.fillSprite(0);  // witte achtergrond

  if (this->writer_ != nullptr) {
    ESP_LOGD(TAG, "Calling display lambda...");
    this->writer_(*this);
    ESP_LOGD(TAG, "Display lambda done.");
  }

  this->canvas_.pushSprite(0, 0);  // push naar het e-paper scherm
  M5.Display.display();            // refresh
  M5.Display.waitDisplay();
  ESP_LOGD(TAG, "M5GFX display update finished.");
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
  this->draw_absolute_pixel_internal(x, y, color);
}

void M5PaperS3DisplayM5GFX::fill(Color color) {
  uint8_t gray = get_native_m5gfx_color_(color);
  this->canvas_.fillSprite(gray);
}

void M5PaperS3DisplayM5GFX::draw_absolute_pixel_internal(int x, int y, Color color) {
  uint8_t gray = get_native_m5gfx_color_(color);
  this->canvas_.drawPixel(x, y, gray);
}

int M5PaperS3DisplayM5GFX::get_width_internal() {
  return M5.Display.width();
}

int M5PaperS3DisplayM5GFX::get_height_internal() {
  return M5.Display.height();
}

void M5PaperS3DisplayM5GFX::set_rotation(int rotation) {
  int m5gfx_rotation = 0;
  switch (rotation) {
    case 90: m5gfx_rotation = 1; break;
    case 180: m5gfx_rotation = 2; break;
    case 270: m5gfx_rotation = 3; break;
    default: m5gfx_rotation = 0; break;
  }
  this->rotation_ = m5gfx_rotation;
}

uint8_t M5PaperS3DisplayM5GFX::get_native_m5gfx_color_(Color color) {
  float r_f = color.r / 255.0f;
  float g_f = color.g / 255.0f;
  float b_f = color.b / 255.0f;

  float gray_f = (r_f * 0.2126f + g_f * 0.7152f + b_f * 0.0722f);
  return static_cast<uint8_t>(gray_f * 15.0f);  // ✅ 0–15 voor 4-bit grijswaarden
}

}  // namespace m5papers3_display_m5gfx
}  // namespace esphome
