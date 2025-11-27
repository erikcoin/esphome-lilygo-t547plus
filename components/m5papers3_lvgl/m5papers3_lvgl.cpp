#include "m5papers3_lvgl.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/color.h"
#include <cmath>
#include <algorithm>
#include <esp_heap_caps.h>
#include "esphome/core/helpers.h"  // voor set_timeout
namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";
// Fast RGB565 → 8-bit luminance converter
static inline uint8_t rgb565_to_luma8(uint16_t c) {
  // fast expand and weighted luma (integer)
  uint8_t r5 = (c >> 11) & 0x1F;
  uint8_t g6 = (c >> 5) & 0x3F;
  uint8_t b5 = c & 0x1F;
  uint8_t r8 = (r5 * 527 + 23) >> 6;
  uint8_t g8 = (g6 * 259 + 33) >> 6;
  uint8_t b8 = (b5 * 527 + 23) >> 6;
  // approximate luma: (0.299 * r + 0.587 * g + 0.114 * b)
  return (uint8_t)((r8 * 77 + g8 * 150 + b8 * 29) >> 8); // normalized
}

static inline uint16_t gray8_to_rgb565(uint8_t g8) {
  // convert 8-bit gray to rgb565
  uint16_t r = (g8 >> 3) & 0x1F;
  uint16_t gg = (g8 >> 2) & 0x3F;
  uint16_t b = (g8 >> 3) & 0x1F;
  return (r << 11) | (gg << 5) | b;
}
static inline uint8_t rgb565_to_gray(uint16_t c) {
    uint8_t r = (c >> 11) & 0x1F;
    uint8_t g = (c >> 5)  & 0x3F;
    uint8_t b =  c        & 0x1F;

    // Expand to 8-bit using integer multipliers (no floats)
    r = (r * 527 + 23) >> 6;
    g = (g * 259 + 33) >> 6;
    b = (b * 527 + 23) >> 6;

    // Standard luma
    return (uint8_t)((r * 38 + g * 75 + b * 15) >> 7);
}
static inline uint16_t gray4_to_rgb565(uint8_t g4) {
    // Expand 4-bit to 8-bit
    uint8_t g8 = (g4 << 4) | g4;
    uint8_t r = g8 >> 3;
    uint8_t g = g8 >> 2;
    uint8_t b = g8 >> 3;
    return (r << 11) | (g << 5) | b;
}

static const uint16_t screenWidth  = 960;
static const uint16_t screenHeight = 540;

lv_disp_draw_buf_t draw_buf;
lv_color_t buf[screenWidth * 10];

void M5PaperS3DisplayM5GFX::my_disp_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
  const u_long w = area->x2 - area->x1 + 1;
  const u_long h = area->y2 - area->y1 + 1;

  gfx_.startWrite();
  gfx_.setAddrWindow(area->x1, area->y1, w, h);
  gfx_.pushColors(static_cast<uint16_t *>(&color_p->full), w * h, true);
  gfx_.endWrite();

  lv_disp_flush_ready(disp);
}


void M5PaperS3DisplayM5GFX::setup() {
  ESP_LOGD(TAG, "M5PaperS3DisplayM5GFX::setup() start");
   auto cfg = M5.config();
   M5.begin(cfg);
   lv_init();
 
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  static lv_disp_drv_t disp_drv;
  M5PaperS3DisplayM5GFX::lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = M5PaperS3DisplayM5GFX::my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  M5PaperS3DisplayM5GFX::lv_disp_drv_register(&disp_drv);

  /* Create simple label */
  lv_obj_t *label = M5PaperS3DisplayM5GFX::lv_label_create(::lv_scr_act());
  M5PaperS3DisplayM5GFX::lv_label_set_text(label, "Aikatsu! 10th anniversary.");
  M5PaperS3DisplayM5GFX::lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

  
// ... (update() method remains largely the same)
void M5PaperS3DisplayM5GFX::update() {
// Intentionally empty so LVGL runs on its dedicated task
  // Keep this function very short — it's called from loopTask (core 1)
  // Optionally: tiny diagnostic every few seconds
  static uint32_t last_log = 0;
  uint32_t now = millis();
  if (now - last_log > 5000) {
    ESP_LOGD(TAG, "update() heartbeat");
    last_log = now;
  }
}

M5PaperS3DisplayM5GFX::~M5PaperS3DisplayM5GFX() {

}


void M5PaperS3DisplayM5GFX::set_rotation(int rotation_degrees) {
    int m5gfx_rotation_val = 0; // 0: 0, 1: 90, 2: 180, 3: 270
    if (rotation_degrees == 90) m5gfx_rotation_val = 1;
    else if (rotation_degrees == 180) m5gfx_rotation_val = 2;
    else if (rotation_degrees == 270) m5gfx_rotation_val = 3;
    this->rotation_ = m5gfx_rotation_val; // Store our logical rotation
    ESP_LOGD(TAG, "set_rotation: degrees=%d mapped to M5GFX rotation=%d", rotation_degrees, m5gfx_rotation_val);
}

void M5PaperS3DisplayM5GFX::loop() {
  lv_timer_handler();
  delay(5);

}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, Color color) {
    // bounds check
    if (x < 0 || y < 0 ||
        x >= this->get_width() ||
        y >= this->get_height()) {
        return;
    }

    // Convert ESPHome Color → RGB565
    uint16_t c = ((color.red & 0xF8) << 8) |
                 ((color.green & 0xFC) << 3) |
                 (color.blue >> 3);

    // Draw using M5GFX
    ESP_LOGD(TAG, "Pushing image %d %e %F",x ,y ,c);
    this->gfx_.drawPixel(x, y, c);
}

int M5PaperS3DisplayM5GFX::get_width_internal() {
     return  this->gfx_.width();
}

int M5PaperS3DisplayM5GFX::get_height_internal() {
    return this->gfx_.height();
}
} // namespace m5papers3_display_m5gfx
} // namespace esphome
