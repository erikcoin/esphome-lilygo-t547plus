#include "m5papers3_lvgl.h"

namespace esphome {
namespace m5papers3_lvgl {

using namespace std;

M5PaperS3LVGL::M5PaperS3LVGL(uint16_t width, uint16_t height, uint16_t buf_lines)
    : width_(width), height_(height), buf_lines_(buf_lines), buf1_(nullptr) {}

void M5PaperS3LVGL::setup() {
  ESP_LOGCONFIG("m5papers3_lvgl", "Setting up M5PaperS3 LVGL component (w=%d h=%d buf_lines=%d)",
                width_, height_, buf_lines_);

  // Ensure M5 is started (if you call M5.begin() somewhere else, it's ok)
  // If your project already calls M5.begin() elsewhere, you can remove this call.
  // M5.begin();

  // Initialize LVGL library (if not already)
  lv_init();

  // Allocate LVGL buffer
  size_t buf_size = static_cast<size_t>(width_) * static_cast<size_t>(buf_lines_);
  buf1_ = (lv_color_t *)malloc(buf_size * sizeof(lv_color_t));
  if (!buf1_) {
    ESP_LOGE("m5papers3_lvgl", "Failed to allocate LVGL buffer (size=%d)", (int)buf_size);
    return;
  }
  lv_disp_draw_buf_init(&draw_buf_, buf1_, nullptr, buf_size);

  // Initialize display driver structure
  lv_disp_drv_init(&disp_drv_);
  disp_drv_.hor_res = width_;
  disp_drv_.ver_res = height_;
  disp_drv_.draw_buf = &draw_buf_;

  // set the flush callback to a static lambda that calls our instance method
  disp_drv_.flush_cb = [](lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
    auto *self = static_cast<M5PaperS3LVGL *>(drv->user_data);
    if (self)
      self->lvgl_flush(area, color_p);
  };
  disp_drv_.user_data = this;

  lv_disp_drv_register(&disp_drv_);

  // Optional: create a default test label so users can verify it's working
  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "LVGL -> M5PaperS3");
  lv_obj_center(label);
}

void M5PaperS3LVGL::loop() {
  // Call LVGL handler periodically. We run it every loop call here.
  // On e-paper you SHOULD NOT call this extremely frequently — but lv_timer_handler is cheap
  // if there are no timers/actions; the heavy part is the flush (performed only when needed).
  lv_timer_handler();

  // small delay-conscious yield (ESPHome will handle actual task scheduling)
  delay(1);
}

void M5PaperS3LVGL::dump_config() {
  ESP_LOGCONFIG("m5papers3_lvgl", "M5PaperS3 LVGL component:");
  ESP_LOGCONFIG("m5papers3_lvgl", "  Width: %d  Height: %d  Buf lines: %d", width_, height_, buf_lines_);
}

inline uint8_t M5PaperS3LVGL::rgb565_to_gray4(uint16_t color565) {
  // Extract RGB 5/6/5 components
  uint8_t r5 = (color565 >> 11) & 0x1F;
  uint8_t g6 = (color565 >> 5) & 0x3F;
  uint8_t b5 = (color565) & 0x1F;

  // Convert to 8-bit per channel approximations
  uint8_t r8 = (r5 * 527 + 23) >> 6;   // approximate expansion
  uint8_t g8 = (g6 * 259 + 33) >> 6;
  uint8_t b8 = (b5 * 527 + 23) >> 6;

  // Compute luminance (standard Rec. 601)
  uint16_t lum = (uint16_t)((299 * r8 + 587 * g8 + 114 * b8) / 1000);

  // Map 0..255 to 0..15 (4-bit grayscale)
  uint8_t gray4 = (lum * 15 + 127) / 255;
  return gray4;  // 0 = black, 15 = white (adjust if your epaper inverts)
}

void M5PaperS3LVGL::lvgl_flush(const lv_area_t *area, lv_color_t *color_p) {
  if (!area || !color_p) {
    lv_disp_flush_ready(&disp_drv_);
    return;
  }

  int32_t x1 = area->x1;
  int32_t y1 = area->y1;
  int32_t x2 = area->x2;
  int32_t y2 = area->y2;

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 >= (int32_t)width_) x2 = width_ - 1;
  if (y2 >= (int32_t)height_) y2 = height_ - 1;

  const int w = x2 - x1 + 1;
  const int h = y2 - y1 + 1;

  // NOTE: Depending on your M5GFX API and display driver, a per-pixel loop might be slow for big areas.
  // Consider batching with pushImage if available. For now we do per-pixel writes then call display() for e-paper.
  for (int yy = 0; yy < h; yy++) {
    for (int xx = 0; xx < w; xx++) {
      // lv_color_t .full contains RGB565 if LV_COLOR_DEPTH == 16
      uint16_t c565 = color_p->full;
      uint8_t gray4 = rgb565_to_gray4(c565);

      // Convert 4-bit gray to a 16-level color that M5GFX expects.
      // M5GFX for e-paper typically expects a 0..15 intensity value when drawing grayscale frames.
      // But most common simple path: map gray4 to 0..255 and call drawPixel with RGB565 approximated value.
      // We'll expand gray4 to 8-bit and write an RGB565 grayscale pixel.
      uint8_t gray8 = (gray4 * 255) / 15;
      // convert gray8 to RGB565
      uint16_t gray565 = ((gray8 >> 3) << 11) | ((gray8 >> 2) << 5) | (gray8 >> 3);

      // Use M5.Display to draw pixel:
      // If your project uses a different instance name (e.g., gfx or display), change this line.
      M5.Display.drawPixel(x1 + xx, y1 + yy, gray565);

      color_p++;
    }
  }

  // After drawing the block we must flush/present to the e-paper display.
  // On e-paper you probably have to call a display() or push update function.
  // Common M5GFX method is `display()` or `update()`; adjust if different.
  // Two options:
  //  - For full update: M5.Display.display();
  //  - For partial updates / faster: M5.EPD->somePartialUpdate(...)
  // We'll call display() here (safe but may be heavier).
  M5.Display.display();

  // Tell LVGL we are ready.
  lv_disp_flush_ready(&disp_drv_);
}

}  // namespace m5papers3_lvgl
}  // namespace esphome
