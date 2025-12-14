#include "lvgaidirect.h"

// include LovyanGFX + M5Unified (adjust if your project uses different header names)
#include <M5Unified.h>        // gives M5.Display and LovyanGFX (M5Unified)
//#include <lgfx/v1.h>         // LGFX Sprite type
#include <atomic>

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *TAG = "m5papers3.display_m5gfx";
uint8_t* linebuf_ = nullptr;
M5PaperS3DisplayM5GFX::~M5PaperS3DisplayM5GFX() {
  // free canvas if created
////  if (this->canvas_) {
////    this->canvas_->deleteSprite();
////    delete this->canvas_;
 ////   this->canvas_ = nullptr;
////  }
  // don't call M5.Display.display() here
}

void M5PaperS3DisplayM5GFX::setup() {
  ESP_LOGD(TAG, "M5PaperS3DisplayM5GFX::setup() start");

  // Init M5 hardware
  auto cfg = M5.config();
  cfg.internal_imu = false;
  M5.begin(cfg);
  vTaskDelay(pdMS_TO_TICKS(100));
M5.Display.setColorDepth(4);  // 4-bit grayscale
M5.Display.setEpdMode(epd_mode_t::epd_quality);
  // Create framebuffer (4-bit grayscale => 2 pixels per byte)
  this->fb_width_  = this->get_width();
  this->fb_height_ = this->get_height();
linebuf_ = (uint8_t*)heap_caps_malloc(fb_width_, MALLOC_CAP_8BIT);

  size_t fb_bytes = (fb_width_ * fb_height_) / 2;

  this->framebuffer_ = (uint8_t*) heap_caps_malloc(fb_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
//bool isEnabled(void) const { return _imu != imu_none; }
  if (!this->framebuffer_) {
    ESP_LOGE(TAG, "Failed to allocate framebuffer in PSRAM!");
    this->initialized_ = false;
    return;
  }

  memset(framebuffer_, 0xFF, fb_bytes);   // fill white (0xF)

  this->initialized_ = true;
  this->dirty_.store(true);

  ESP_LOGI(TAG, "Framebuffer allocated: %dx%d, %u bytes (PSRAM free=%u)",
           fb_width_, fb_height_, fb_bytes,
           heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

  // Touch registration (same as before)
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = [](lv_indev_drv_t *drv, lv_indev_data_t *data) {
    auto *comp = static_cast<M5PaperS3DisplayM5GFX *>(drv->user_data);
    data->point.x = comp->last_touch_x_;
    data->point.y = comp->last_touch_y_;
    data->state   = comp->last_touch_pressed_ ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
  };
  indev_drv.user_data = this;
  lv_indev_drv_register(&indev_drv);

    if (!M5.Imu.isEnabled()) {
        ESP_LOGW(TAG, "IMU not enabled");
    } else {
        ESP_LOGI(TAG, "Gyro/Accelerator initialized.");
    }


}

void M5PaperS3DisplayM5GFX::update() {
  if (!initialized_) return;

  if (dirty_.exchange(false)) {
    flush_framebuffer_to_display();
  }

  poll_touch();
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
  if (!initialized_) return;
  if (x < 0 || y < 0 || x >= fb_width_ || y >= fb_height_) return;

  uint8_t idx = color_to_gray4(color);
  draw_pixel_internal_at(x, y, idx);

  dirty_.store(true);
}

uint8_t M5PaperS3DisplayM5GFX::color_to_gray4(const esphome::Color &c) {
  // sRGB luma approximation
  uint8_t r = c.red;
  uint8_t g = c.green;
  uint8_t b = c.blue;
  // integer luminance 0..255
  uint16_t lum = (uint16_t)((299 * r + 587 * g + 114 * b) / 1000);
  // map to 0..15 (0=black, 15=white)
  uint8_t idx = (uint8_t)((lum * 15 + 127) / 255);
  return idx & 0x0F;
}

void M5PaperS3DisplayM5GFX::poll_touch() {
  static int64_t last_touch_time = 0;
  const int DEBOUNCE_MS = 850;  // adjust to taste

  if (!M5.Touch.isEnabled()) return;
  // Check how many touch points are active
  uint8_t count = M5.Touch.getCount();
  if (count > 0) {
    // Get the first touch point
    auto p = M5.Touch.getDetail(0);
    int64_t now = esp_timer_get_time() / 1000;  // microseconds → ms
    if (now - last_touch_time > DEBOUNCE_MS) {
    ESP_LOGD(TAG, "Touch at (%d,%d) pressed=%d", p.x, p.y, p.isPressed());
    this->last_touch_x_ = p.x;
    this->last_touch_y_ = p.y;
    this->last_touch_pressed_ = p.isPressed();

    last_touch_time = now;
    }
   
  } else {
    
    last_touch_pressed_ = false;

  }
}

void M5PaperS3DisplayM5GFX::draw_pixel_internal_at(int x, int y, uint8_t idx) {
  // 4-bit mode: two pixels per byte
  size_t index = y * fb_width_ + x;
  size_t byte_index = index >> 1;

  if (index & 1) {
    // odd pixel => low nibble
    framebuffer_[byte_index] = (framebuffer_[byte_index] & 0xF0) | (idx & 0x0F);
  } else {
    // even pixel => high nibble
    framebuffer_[byte_index] = (framebuffer_[byte_index] & 0x0F) | (idx << 4);
  }
}
void M5PaperS3DisplayM5GFX::flush_framebuffer_to_display() {
    const int w = fb_width_;
    const int h = fb_height_;

    M5.Display.startWrite();

    for (int y = 0; y < h; y++) {
        // --- Convert one line from 4-bit → 8-bit grayscale ---
        int fb_index = (y * w) >> 1;  // byte index in packed FB

        for (int x = 0; x < w; x++) {
            uint8_t byte = framebuffer_[fb_index];

            uint8_t gray4;
            if (x & 1) {
                gray4 = byte & 0x0F;
                fb_index++;
            } else {
                gray4 = byte >> 4;
            }

            // convert to full 0–255 range
            linebuf_[x] = (gray4 * 255) / 15;
        }

        // push converted line
        M5.Display.pushImage(0, y, w, 1, linebuf_);
    }

    M5.Display.endWrite();
}

void M5PaperS3DisplayM5GFX::loop() {
  if (!this->initialized_) return;
  vTaskDelay(pdMS_TO_TICKS(500));
  M5.update();
  poll_touch();
 
}


}  // namespace m5papers3_display_m5gfx
}  // namespace esphome
