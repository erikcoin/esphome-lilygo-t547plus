#include "lvglai2.h"

namespace esphome {
namespace m5papers3ns {

void M5PaperS3Display::setup() {
  ESP_LOGI("m5paper", "Initializing M5Paper S3 display...");

  auto cfg = M5.config();
  M5.begin(cfg);

  gfx_ = &M5.Display;

  // prepare framebuffer: 4-bit grayscale (2 pixels per byte)
  int total_pixels = this->get_width() * this->get_height();
  fb_.assign(total_pixels, 0xFF);  // white background

  gfx_->init();
  gfx_->setEpdMode(epd_mode_t::epd_quality);  // faster refresh

  ESP_LOGI("m5paper", "M5Paper S3 display initialized.");
}

void M5PaperS3Display::dump_config() {
  ESP_LOGCONFIG("m5paper", "M5Paper S3 Display:");
  ESP_LOGCONFIG("m5paper", "  Resolution: %dx%d", this->get_width(), this->get_height());
}

void M5PaperS3Display::update() {
  if (dirty_.exchange(false)) {
    flush_();
  }
}

void M5PaperS3Display::draw_pixel_at(int x, int y, Color color) {
  if ((unsigned)x >= this->get_width()) return;
  if ((unsigned)y >= this->get_height()) return;

  // convert to 4-bit grayscale
  uint8_t idx = (color.r * 38 + color.g * 75 + color.b * 15) >> 8;  // 0–255
  idx >>= 4; // convert to 0–15

  draw_absolute_pixel_internal(x, y, idx);
  dirty_.store(true);
}

void M5PaperS3Display::flush_() {
  // M5GFX accepts 4-bit data directly:
  gfx_->pushImage(
      0,
      0,
      this->get_width(),
      this->get_height(),
      fb_.data(),
      lgfx::grayscale_4bit




  );
}
};
};
