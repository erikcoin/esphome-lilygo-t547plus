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


// Build once at startup (e.g., in setup())
static uint8_t  lut_rgb565_to_luma[1 << 16];
static uint16_t lut_gray8_to_rgb565[256];

static inline uint8_t rgb565_to_luma(uint16_t c)
{
  uint8_t r5 = (c >> 11) & 0x1F;
  uint8_t g6 = (c >> 5)  & 0x3F;
  uint8_t b5 =  c        & 0x1F;
  uint8_t r = (r5 * 527 + 23) >> 6;
  uint8_t g = (g6 * 259 + 33) >> 6;
  uint8_t b = (b5 * 527 + 23) >> 6;
  return (uint8_t)((r * 77 + g * 150 + b * 29) >> 8);
}

static inline uint16_t gray8_to_rgb565(uint8_t y)
{
  uint16_t r5 = (y * 31 + 127) / 255;
  uint16_t g6 = (y * 63 + 127) / 255;
  uint16_t b5 = r5;
  return (uint16_t)((r5 << 11) | (g6 << 5) | b5);
}

static void init_luts()
{
  for (uint32_t c = 0; c < (1u << 16); ++c) {
    lut_rgb565_to_luma[c] = rgb565_to_luma((uint16_t)c);
  }
  for (int y = 0; y < 256; ++y) {
    lut_gray8_to_rgb565[y] = gray8_to_rgb565((uint8_t)y);
  }
}






//static inline uint16_t gray8_to_rgb565(uint8_t g8) {
  // convert 8-bit gray to rgb565
//  uint16_t r = (g8 >> 3) & 0x1F;
//  uint16_t gg = (g8 >> 2) & 0x3F;
//  uint16_t b = (g8 >> 3) & 0x1F;
//  return (r << 11) | (gg << 5) | b;
//}
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

  // LVGL init
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
  const int LV_BUF_LINES = 20;  // tune: smaller -> smaller heap usage, more flushes
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
//  linebuf_capacity_ = static_cast<size_t>(w); // one uint16_t per pixel per line
  ESP_LOGD(TAG, "Allocating line buffers in PSRAM: %u pixels", (unsigned)linebuf_capacity_);
  linebufA_ = (uint16_t*) heap_caps_malloc(linebuf_capacity_ * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
  linebufB_ = (uint16_t*) heap_caps_malloc(linebuf_capacity_ * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
  if (!linebufA_ || !linebufB_) {
    ESP_LOGE(TAG, "Failed to allocate PSRAM line buffers: A=%p B=%p", (void*)linebufA_, (void*)linebufB_);
    if (linebufA_) heap_caps_free(linebufA_), linebufA_ = nullptr;
    if (linebufB_) heap_caps_free(linebufB_), linebufB_ = nullptr;
    // still continue — flush will guard against null buffers
  }

  ESP_LOGD(TAG, "LVGL setup complete. Free PSRAM: %u", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));



////if (r != pdPASS) {
////    ESP_LOGE(TAG, "Failed to create flush worker task!");
////}



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

  ESP_LOGD(TAG, "starting loop, starting update now");
  
 //   M5.update(); // Update touch and other inputs
    ESP_LOGD(TAG, "in loop after update ");
    static unsigned long last_touch_time = 0;
unsigned long current_time = millis();

}

void M5PaperS3DisplayM5GFX::lvgl_flush(const lv_area_t *area, lv_color_t *color_p)
{
  const int16_t x = area->x1;
  const int16_t y = area->y1;
  const int16_t w = area->x2 - area->x1 + 1;
  const int16_t h = area->y2 - area->y1 + 1;

  if (w <= 0 || h <= 0) {
    lv_disp_flush_ready(&this->disp_drv_);
    return;
  }

  // Use your existing PSRAM line buffers if available; else fallback to chunk alloc.
  // Each line buffer holds 'linebuf_capacity_' pixels as defined in your class.
  const int pitch = w;

  // Choose chunk height (tune as needed; 120 is usually good on PSRAM)
  int chunk_lines = 120;

  // If your line buffers are smaller than one chunk, limit to their capacity
  size_t max_pixels_per_buf = linebuf_capacity_;  // number of pixels each line buffer can hold
  if (max_pixels_per_buf > 0) {
    size_t max_lines_by_buf = max_pixels_per_buf / (size_t)pitch;
    if (max_lines_by_buf == 0) {
      // Buffers too small for full width; revert to per-line conversion
      chunk_lines = 1;
    } else {
      chunk_lines = (int)std::min((size_t)chunk_lines, max_lines_by_buf);
    }
  }

  // Determine buffers to use
  uint16_t* bufA = linebufA_;
  uint16_t* bufB = linebufB_;
  bool have_double = (bufA != nullptr) && (bufB != nullptr) && (chunk_lines > 0);

  // Fallback: allocate temporary chunks if class buffers are not available
  auto alloc_fast = [&](size_t bytes) -> uint16_t* {
    return (uint16_t*)heap_caps_malloc(bytes, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
  };

  size_t chunk_pixels = (size_t)pitch * (size_t)chunk_lines;
  size_t chunk_bytes  = chunk_pixels * sizeof(uint16_t);

  if (!have_double) {
    bufA = alloc_fast(chunk_bytes);
    bufB = alloc_fast(chunk_bytes);
    if (!bufA || !bufB) {
      // Try smaller chunk
      if (bufA) heap_caps_free(bufA);
      if (bufB) heap_caps_free(bufB);
      chunk_lines = 40;
      chunk_pixels = (size_t)pitch * (size_t)chunk_lines;
      chunk_bytes  = chunk_pixels * sizeof(uint16_t);
      bufA = alloc_fast(chunk_bytes);
      bufB = alloc_fast(chunk_bytes);
    }
    have_double = (bufA && bufB);
  }

  // If we still don't have double buffers, fall back to line-by-line (still LUT-accelerated)
  if (!have_double) {
    for (int row = 0; row < h; ++row) {
      // Convert one line into a small temp
      uint16_t* line = alloc_fast(pitch * sizeof(uint16_t));
      if (!line) line = (uint16_t*)malloc(pitch * sizeof(uint16_t));
      lv_color_t* src = color_p + row * pitch;
      for (int col = 0; col < pitch; ++col) {
        uint16_t c565 = src[col].full;
        uint8_t  lum  = lut_rgb565_to_luma[c565];
        line[col]     = lut_gray8_to_rgb565[lum];
      }
      gfx_.pushImage(x, y + row, pitch, 1, line);  // blocking push when DMA not used
      if (line) heap_caps_free(line);
      if ((row & 0x1F) == 0) vTaskDelay(1);
    }
    lv_disp_flush_ready(&this->disp_drv_);
    return;
  }

  // Double-buffered DMA path
  uint16_t* dma_buf[2] = { bufA, bufB };
  int buf_idx = 0;

  int remaining_rows = h;
  int processed_rows = 0;

  // Helper to convert N lines into dst using LUTs
  auto convert_lines = [&](uint16_t* dst, lv_color_t* src, int lines) {
    size_t pixels = (size_t)pitch * (size_t)lines;
    for (size_t i = 0; i < pixels; ++i) {
      uint16_t c565 = src[i].full;
      uint8_t  lum  = lut_rgb565_to_luma[c565];
      dst[i]        = lut_gray8_to_rgb565[lum];
    }
  };

  // Prepare and kick off first DMA block
  int lines_this_chunk = std::min(remaining_rows, chunk_lines);
  convert_lines(dma_buf[buf_idx], color_p + processed_rows * pitch, lines_this_chunk);
  gfx_.pushImageDMA(x, y + processed_rows, pitch, lines_this_chunk, dma_buf[buf_idx]);

  remaining_rows -= lines_this_chunk;
  processed_rows += lines_this_chunk;
  buf_idx ^= 1;

  while (remaining_rows > 0) {
    lines_this_chunk = std::min(remaining_rows, chunk_lines);

    // Convert next chunk while previous DMA is in-flight
    convert_lines(dma_buf[buf_idx], color_p + processed_rows * pitch, lines_this_chunk);

    // Ensure previous DMA completed
    gfx_.waitDMA();

    // Start next DMA transfer
    gfx_.pushImageDMA(x, y + processed_rows, pitch, lines_this_chunk, dma_buf[buf_idx]);

    remaining_rows -= lines_this_chunk;
    processed_rows += lines_this_chunk;
    buf_idx ^= 1;

    // Keep system responsive
    vTaskDelay(1);
  }

  // Final DMA completion
  gfx_.waitDMA();

  // If we allocated temporary buffers, free them
  if (linebufA_ != bufA) { if (bufA) heap_caps_free(bufA); }
  if (linebufB_ != bufB) { if (bufB) heap_caps_free(bufB); }

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
