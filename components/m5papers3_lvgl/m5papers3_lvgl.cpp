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
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
  if (!drv) return;
  if (!drv->user_data) {
    // best effort: mark flush ready if LVGL calls us but no userdata
    lv_disp_flush_ready(drv);
    return;
  }
  auto *display = static_cast<esphome::m5papers3_display_m5gfx::M5PaperS3DisplayM5GFX*>(drv->user_data);
  if (!display) {
    lv_disp_flush_ready(drv);
    return;
  }
  display->lvgl_flush(area, color_p);
}

// ... (M5PaperS3DisplayM5GFX::setup() remains largely the same, ensure logging is as you need it)
void M5PaperS3DisplayM5GFX::setup() {
  ESP_LOGD(TAG, "M5PaperS3DisplayM5GFX::setup() start");
  // M5 init (keep your existing sequence; make sure M5.begin() already ran)
  auto cfg = M5.config();
  M5.begin(cfg);
  vTaskDelay(pdMS_TO_TICKS(100));
  lv_init();
xTaskCreatePinnedToCore(
    &M5PaperS3DisplayM5GFX::flush_worker_task_trampoline,
    "lv_flush_worker",
    8192,       // stack
    this,       // parameter
    1,          // priority
    &flush_task_handle_,
    1           // run on core 1
);

  const int w = this->get_width();
  const int h = this->get_height();
  ESP_LOGD(TAG, "Display size %d x %d", w, h);

  // LVGL draw buffers: allocate in PSRAM
  const int LV_BUF_LINES = 80;  // tune: smaller -> smaller heap usage, more flushes
  const size_t buf_size = (size_t)w * LV_BUF_LINES;

  ESP_LOGD(TAG, "Allocating LVGL draw buffers in PSRAM: %u pixels each", (unsigned)buf_size);
  lv_buf1_ = (lv_color_t*) heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
  lv_buf2_ = (lv_color_t*) heap_caps_malloc(buf_size * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
  if (!lv_buf1_ || !lv_buf2_) {
    ESP_LOGE(TAG, "Failed to allocate LVGL buffers in PSRAM! buf1=%p buf2=%p", (void*)lv_buf1_, (void*)lv_buf2_);
    // free any partial allocations
    if (lv_buf1_) heap_caps_free(lv_buf1_), lv_buf1_ = nullptr;
    if (lv_buf2_) heap_caps_free(lv_buf2_), lv_buf2_ = nullptr;
    return;
  }
  lv_disp_draw_buf_init(&draw_buf_, lv_buf1_, lv_buf2_, buf_size);

  // Register LVGL display driver
  lv_disp_drv_init(&disp_drv_);
  disp_drv_.hor_res = w;
  disp_drv_.ver_res = h;
  disp_drv_.draw_buf = &draw_buf_;
  disp_drv_.flush_cb = lvgl_flush_cb;
  disp_drv_.user_data = this;
  lv_disp_t *disp = lv_disp_drv_register(&disp_drv_);
  if (!disp) {
    ESP_LOGE(TAG, "lv_disp_drv_register failed");
    // free buffers before returning
    heap_caps_free(lv_buf1_); lv_buf1_ = nullptr;
    heap_caps_free(lv_buf2_); lv_buf2_ = nullptr;
    return;
  }
  // Some LVGL versions keep driver behind disp->driver pointer; ensure user_data mirrored
  if (disp->driver) disp->driver->user_data = this;

  // Allocate two persistent PSRAM line buffers (RGB565 words) — allocated once
  const int CHUNK_LINES = 80; // tune based on PSRAM
  linebuf_capacity_ = (size_t)w * CHUNK_LINES;
  ESP_LOGD(TAG, "Allocating line buffers in PSRAM: %u pixels", (unsigned)linebuf_capacity_);
  linebufA_ = (uint16_t*) heap_caps_malloc(linebuf_capacity_ * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
  linebufB_ = (uint16_t*) heap_caps_malloc(linebuf_capacity_ * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
  if (!linebufA_ || !linebufB_) {
    ESP_LOGE(TAG, "Failed to allocate PSRAM line buffers: A=%p B=%p", (void*)linebufA_, (void*)linebufB_);
    if (linebufA_) heap_caps_free(linebufA_), linebufA_ = nullptr;
    if (linebufB_) heap_caps_free(linebufB_), linebufB_ = nullptr;
  }
  ESP_LOGD(TAG, "LVGL setup complete. Free PSRAM: %u", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
}

// ... (update() method remains largely the same)
void M5PaperS3DisplayM5GFX::update() {
  // Throttle lv_timer_handler() frequency to avoid overloading LVGL and the CPU
  const uint32_t LVGL_TICK_INTERVAL_MS = 2000; // try 20ms (50Hz). Lower if necessary.
  uint32_t now = millis();

  // run heap/psram diagnostics every ~5s only (avoid spam)
  static uint32_t last_diag = 0;
  if (now - last_diag > 5000) {
    ESP_LOGD(TAG, "Heap internal: %u  PSRAM: %u",
             heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
             heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    last_diag = now;
  }

  // throttle timer
  if (now - this->lvgl_last_tick_ms_ < LVGL_TICK_INTERVAL_MS) return;
  this->lvgl_last_tick_ms_ = now;

  // prevent re-entrancy if previous lv_timer_handler not finished
  if (this->lvgl_busy_.exchange(true)) {
    // Another lvgl handler still running; skip this tick
    ESP_LOGW(TAG, "Skipping lv_timer_handler() because busy");
    return;
  }
  // Run LVGL timer handler safely; make sure we catch unexpected issues quickly
  // (Note: no exceptions in ESP32, but we keep the handler brief)
  ESP_LOGD(TAG, "before lv_timer_handler.");
  lv_timer_handler();
ESP_LOGD(TAG, "after  lv_timer_handler.");
  // mark not busy
  this->lvgl_busy_.store(false);
  ESP_LOGD(TAG, "after  lvgl_busy_.store.");
}

M5PaperS3DisplayM5GFX::~M5PaperS3DisplayM5GFX() {

}

void M5PaperS3DisplayM5GFX::flush_worker_task_trampoline(void *param) {
  static_cast<M5PaperS3DisplayM5GFX*>(param)->flush_worker_task();
}

void M5PaperS3DisplayM5GFX::flush_worker_task() {
  while (true) {
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(5));  // do NOT use 1ms on S3
  }
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
  ESP_LOGD(TAG, "starting loop");
 //   M5.update(); // Update touch and other inputs
 //   ESP_LOGD(TAG, "in loop after update ");
 //   static unsigned long last_touch_time = 0;
//unsigned long current_time = millis();

}

void M5PaperS3DisplayM5GFX::lvgl_flush(const lv_area_t *area, lv_color_t *color_p) {
  static uint32_t last_refresh = 0;
  uint32_t now = millis();
//
  // Rate-limit partial updates (VERY IMPORTANT for e-paper)
  //
  if (now - last_refresh < 1000) {
    lv_disp_flush_ready(&this->disp_drv_);
    return;
  }
  last_refresh = now;

  // Defensive
  if (!area || !color_p) {
    ESP_LOGD(TAG, "lvgl_flush QUEUED; returning to LVGL");
    lv_disp_flush_ready(&this->disp_drv_);
    return;
  }

  if (!disp_drv_.user_data) {
    ESP_LOGE(TAG, "lvgl_flush: disp_drv_.user_data is NULL");
    lv_disp_flush_ready(&this->disp_drv_);
    return;
  }

  //
  // Clip to display area
  //
  const int x1 = std::max<int>(area->x1, 0);
  const int y1 = std::max<int>(area->y1, 0);
  const int x2 = std::min<int>(area->x2, this->get_width()  - 1);
  const int y2 = std::min<int>(area->y2, this->get_height() - 1);

  const int w = x2 - x1 + 1;
  const int h = y2 - y1 + 1;

  if (w <= 0 || h <= 0) {
    ESP_LOGD(TAG, "flush ready regel 233");
    lv_disp_flush_ready(&this->disp_drv_);
    return;
  }

  //
  // --- Fallback when persistent buffers failed ---
  //
  if (!linebufA_ || !linebufB_) {
    ESP_LOGW(TAG, "lvgl_flush: fallback stack buffer (no PSRAM buffers)");

    if (w > 512) {  // safe stack limit
      ESP_LOGE(TAG, "lvgl_flush: fallback buffer too large for stack (w=%d)", w);
      lv_disp_flush_ready(&this->disp_drv_);
      return;
    }

    uint16_t stackbuf[512];   // static max
    lv_color_t *src = color_p;

    for (int yy = 0; yy < h; yy++) {
      // convert 1 line
      for (int xx = 0; xx < w; xx++) {
        uint16_t c565 = src[xx].full;
        uint8_t lum   = rgb565_to_luma8(c565);
        stackbuf[xx]  = gray8_to_rgb565(lum);
      }
      ESP_LOGD(TAG, "Pushing image %d %e", x1,y1);
      M5.Display.pushImage(x1, y1 + yy, w, 1, stackbuf);
      src += w;

      // Yield to prevent WDT
      delay(0);
    }

    lv_disp_flush_ready(&this->disp_drv_);
    return;
  }

  //
  // --- Main PSRAM double-buffer pipeline ---
  //
  uint16_t *cur  = linebufA_;
  uint16_t *next = linebufB_;
  lv_color_t *src = color_p;

  for (int yy = 0; yy < h; yy++) {

    // convert LVGL → grayscale RGB565
    for (int xx = 0; xx < w; xx++) {
      uint16_t c565 = src[xx].full;
      uint8_t lum   = rgb565_to_luma8(c565);
      cur[xx]       = gray8_to_rgb565(lum);
    }

    // push 1 scanline
    M5.Display.pushImage(x1, y1 + yy, w, 1, cur);

    src += w;

    // swap buffers
    uint16_t *tmp = cur;
    cur = next;
    next = tmp;

    // IMPORTANT: give LVGL & watchdog time to breathe
    delay(0);
  }

  lv_disp_flush_ready(&this->disp_drv_);
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
