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
//bool M5PaperS3DisplayM5GFX::use_lvgl_ = true;
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

//lv_disp_draw_buf_t draw_buf;
//lv_color_t buf[screenWidth * 10];

void M5PaperS3DisplayM5GFX::lvgl_flush_cb_trampoline(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
  if (!drv) return;
  auto *self = static_cast<M5PaperS3DisplayM5GFX *>(drv->user_data);
  if (!self) {
    lv_disp_flush_ready(drv);
    return;
  }
  self->lvgl_flush_cb(area, color_p);
}

void M5PaperS3DisplayM5GFX::lvgl_flush_cb(const lv_area_t *area, lv_color_t *color_p) {
  if (!area || !color_p) {
    lv_disp_flush_ready(&this->disp_drv_);
    return;
  }

  // clip safely
  const int x1 = std::max<int>(area->x1, 0);
  const int y1 = std::max<int>(area->y1, 0);
  const int x2 = std::min<int>(area->x2, this->get_width() - 1);
  const int y2 = std::min<int>(area->y2, this->get_height() - 1);

  const int w = x2 - x1 + 1;
  const int h = y2 - y1 + 1;
  if (w <= 0 || h <= 0) {
    lv_disp_flush_ready(&this->disp_drv_);
    return;
  }

  // color_p points into lv_framebuf_ (LVGL owns it). Push that rectangle to the device buffer:
  M5.Display.pushImage(x1, y1, w, h, (uint16_t *)color_p);

  // IMPORTANT: do not call M5.Display.display() here for every flush.
  // The LVGL task or update loop should call display() once per finished frame.
  lv_disp_flush_ready(&this->disp_drv_);
}

void M5PaperS3DisplayM5GFX::lvgl_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p) {
  const u_long w = area->x2 - area->x1 + 1;
  const u_long h = area->y2 - area->y1 + 1;

  M5.Display.startWrite();
  M5.Display.setAddrWindow(area->x1, area->y1, w, h);
  M5.Display.pushColors(static_cast<uint16_t *>(&color_p->full), w * h, true);
  M5.Display.endWrite();

  lv_disp_flush_ready(disp);
}

void M5PaperS3DisplayM5GFX::lvgl_task_trampoline(void *arg) {
  auto *self = static_cast<M5PaperS3DisplayM5GFX *>(arg);
  if (!self) { vTaskDelete(NULL); return; }
  self->lvgl_task();
}

void M5PaperS3DisplayM5GFX::lvgl_task() {
  const TickType_t interval = pdMS_TO_TICKS(30); // tune 20-50 ms
  for (;;) {
    lv_timer_handler();               // causes LVGL to render and fire flushes
    M5.Display.display();             // present the frame once per timer run
    vTaskDelay(interval);
  }
}


void M5PaperS3DisplayM5GFX::setup() {
  ESP_LOGD(TAG, "M5PaperS3DisplayM5GFX::setup() start");
   auto cfg = M5.config();
   M5.begin(cfg);
   lv_init();
 // lv_disp_draw_buf_init(&draw_buf, buf, NULL, screenWidth * 10);

  
// allocate full framebuffer in PSRAM (recommended)
const int w = this->get_width();
const int h = this->get_height();
size_t fb_pixels = (size_t)w * (size_t)h;
this->lv_framebuf_ = (lv_color_t*) heap_caps_malloc(fb_pixels * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
if (!this->lv_framebuf_) {
  ESP_LOGE(TAG, "LVGL PSRAM allocation failed");
  return;
}
lv_disp_draw_buf_init(&this->draw_buf_, this->lv_framebuf_, nullptr, (int)fb_pixels);

// register driver (store disp_drv_ as member)
lv_disp_drv_init(&this->disp_drv_);
this->disp_drv_.hor_res = w;
this->disp_drv_.ver_res = h;
this->disp_drv_.draw_buf = &this->draw_buf_;
this->disp_drv_.flush_cb = M5PaperS3DisplayM5GFX::lvgl_flush_cb_trampoline;
this->disp_drv_.user_data = this;
lv_disp_t *disp = lv_disp_drv_register(&this->disp_drv_);
if (!disp) {
  ESP_LOGE(TAG, "lv_disp_drv_register failed");
  heap_caps_free(this->lv_framebuf_);
  this->lv_framebuf_ = nullptr;
  return;
}
  xTaskCreatePinnedToCore(&M5PaperS3DisplayM5GFX::lvgl_task_trampoline,
                        "lvgl_task", 4096, this,
                        tskIDLE_PRIORITY + 2, &this->lvgl_task_handle_, 0);

if (disp->driver) disp->driver->user_data = this; // some LVGL builds use this indirection

  /* Create simple label */
lv_obj_t *label = lv_label_create(lv_scr_act());
lv_label_set_text(label, "Hallo LVGL!");
lv_obj_center(label);
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
  delay(105);

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
