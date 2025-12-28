#include "lvgaidirect.h"
#include "esphome/components/api/api_server.h"

#include "esphome/core/helpers.h"
// include LovyanGFX + M5Unified (adjust if your project uses different header names)
#include <M5Unified.h>        // gives M5.Display and LovyanGFX (M5Unified)
//#include <lgfx/v1.h>         // LGFX Sprite type
#include "esphome/components/wifi/wifi_component.h"
#include <atomic>
#include "esp_sleep.h"

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *TAG = "m5papers3.display_m5gfx";
uint8_t* linebuf_ = nullptr;
bool wifi_ready_ = false;
bool api_ready_ = false;
bool post_wakeup_ready_ = false;
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
  //lv_indev_t *indev_;
  //indev_ = lv_indev_drv_register(&indev_drv);
  
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = [](lv_indev_drv_t *drv, lv_indev_data_t *data) {
    auto *comp = static_cast<M5PaperS3DisplayM5GFX *>(drv->user_data);
      // 🚨 HARD BLOCK LVGL DURING WAKE
  //if (comp->suppress_lvgl_input_) {
 //   data->state = LV_INDEV_STATE_REL;
 //   return;
 // }
    data->point.x = comp->last_touch_x_;
    data->point.y = comp->last_touch_y_;
    data->state   = comp->last_touch_pressed_ ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
  };
  indev_drv.user_data = this;
  lv_indev_drv_register(&indev_drv);
last_activity_ = esp_timer_get_time() / 1000;  // ms
    if (!M5.Imu.isEnabled()) {
        ESP_LOGW(TAG, "IMU not enabled");
    } else {
        ESP_LOGI(TAG, "Gyro/Accelerator initialized.");
    }

 
}

void M5PaperS3DisplayM5GFX::update() {
  if (!initialized_) return;
 // if (esphome::api::global_api_server->is_connected()) {
  if (dirty_.exchange(false)) {
    flush_framebuffer_to_display();
  }
//  }
   // this->setup();  // re‑init display + touch
    // Wait until WiFi is connected

  //poll_touch();
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
  if (!initialized_) return;
//    if (!api_ready_) {
//    auto *api = esphome::api::global_api_server;
//    if (api != nullptr && api->is_connected()) {
//      ESP_LOGI(TAG, "API connected");
//      api_ready_ = true;
//    } else {
//      return;
//    }
//  }
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
  const int DEBOUNCE_MS = 850;

  if (!M5.Touch.isEnabled()) return;

  uint8_t count = M5.Touch.getCount();
  int64_t now = esp_timer_get_time() / 1000;  // µs → ms

  if (count > 0) {
    auto p = M5.Touch.getDetail(0);

    // Always update activity timer
    last_activity_ = now;

    // --- Capture wake touch if LVGL input suppressed ---
//    if (suppress_lvgl_input_ && !pending_wake_touch_ && !api_ready_) {
//      pending_wake_touch_ = true;
//      pending_touch_x_ = p.x;
//      pending_touch_y_ = p.y;
//      ESP_LOGD(TAG, "Captured wake touch at (%d,%d)", p.x, p.y);
 //   }

    // Normal debounce + state update
    if (now - last_touch_time > DEBOUNCE_MS) {
      ESP_LOGD(TAG, "Touch at (%d,%d) pressed=%d", p.x, p.y, p.isPressed());
      last_touch_x_ = p.x;
      last_touch_y_ = p.y;
      last_touch_pressed_ = p.isPressed();
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

  // 1. Handle Connectivity first
//  if (!wifi_ready_) {
//    auto *wifi = esphome::wifi::global_wifi_component;
//    if (wifi != nullptr && wifi->is_connected()) {
//      wifi_ready_ = true;
//    } else { return; }
//  }

//  if (!api_ready_) {
//    auto *api = esphome::api::global_api_server;
//    if (api != nullptr && api->is_connected()) {
//      api_ready_ = true;
      // The API is finally back! Now we can allow the touch to be processed.
//    } else { return; }
//  }

  // 2. Replay Logic (Only runs once API is ready)
//  if (suppress_lvgl_input_) {
//    if (pending_wake_touch_) {
//      ESP_LOGI(TAG, "Replaying wake touch to LVGL at (%d,%d)", pending_touch_x_, pending_touch_y_);
      
      // Set the coordinates for the input_read callback
//      last_touch_x_ = pending_touch_x_;
//      last_touch_y_ = pending_touch_y_;
      
      // Signal a press
//      last_touch_pressed_ = true;
      // We don't call lv_timer_handler manually here; 
      // we let the normal ESPHome LVGL component pick it up this frame.
      
      pending_wake_touch_ = false; 
      // We keep suppress_lvgl_input_ = true for ONE more frame 
      // to ensure the 'Release' is registered next.
 //     return; 
 //   } else {
      // If we were suppressing and already replayed the press, 
      // now we release and open the gates.
//      last_touch_pressed_ = false;
//      suppress_lvgl_input_ = false; 
//      ESP_LOGI(TAG, "LVGL input fully unblocked");
//    }
//  }

  M5.update();
  poll_touch();

  // ... rest of your sleep logic ...

  int64_t now = esp_timer_get_time() / 1000;
  if (sleep_duration_ms > 0 && (now - last_activity_) > 45000 ) {
     // ... prep for sleep ...
     esp_sleep_enable_timer_wakeup(sleep_duration_ms * 1000ULL);
     gpio_wakeup_enable((gpio_num_t)touch_gpio, GPIO_INTR_LOW_LEVEL);
     esp_sleep_enable_gpio_wakeup();
  //   suppress_lvgl_input_ = true; 
   //  pending_wake_touch_ = false;

  //  wifi::global_wifi_component->disable();
   
   esp_light_sleep_start();
     
     // After wake:
//     wifi_ready_ = false;
//     api_ready_ = false;
     last_activity_ = esp_timer_get_time() / 1000;
  }
}

}  // namespace m5papers3_display_m5gfx
}  // namespace esphome
