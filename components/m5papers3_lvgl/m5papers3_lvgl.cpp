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

// ... (M5PaperS3DisplayM5GFX::setup() remains largely the same, ensure logging is as you need it)
void M5PaperS3DisplayM5GFX::setup() {
    ESP_LOGD(TAG, "Setting up M5PaperS3 Display...");

    // --- M5 Display init ---
    auto cfg = M5.config();
    M5.begin(cfg);
    //vTaskDelay(pdMS_TO_TICKS(100));
    M5.Display.setEpdMode(epd_mode_t::epd_quality);
   // vTaskDelay(pdMS_TO_TICKS(1000));

    // --- Allocate full-screen PSRAM framebuffer (4-bit grayscale) ---
    epd_buffer_ = (uint8_t*)heap_caps_malloc((this->get_width() * this->get_height()) / 2, MALLOC_CAP_SPIRAM);
    if (!epd_buffer_) {
        ESP_LOGE(TAG, "Failed to allocate PSRAM framebuffer!");
        return;
    }
    memset(epd_buffer_, 0xFF, (this->get_width() * this->get_height()) / 2); // fill white

    // --- LVGL init ---
    lv_init();
        // Prepare driver
    lv_disp_drv_init(&this->disp_drv_);
    this->disp_drv_.hor_res = this->get_width();
    this->disp_drv_.ver_res = this->get_height();
    this->disp_drv_.flush_cb = lvgl_flush_cb;
    this->disp_drv_.user_data = this;        // <-- correct place!
    
    lv_disp_t *disp = lv_disp_drv_register(&this->disp_drv_);
   //// int w = this->get_width();
   //// int h = this->get_height();
   //// size_t buf_size = w * LV_BUF_LINES;

   //// lv_color_t *lv_buf1 = (lv_color_t*)malloc(buf_size * sizeof(lv_color_t));
  ////  lv_color_t *lv_buf2 = (lv_color_t*)malloc(buf_size * sizeof(lv_color_t));
    if (!lv_buf1 || !lv_buf2) {
        ESP_LOGE(TAG, "Failed to allocate LVGL draw buffers");
        return;
    }

    lv_disp_draw_buf_init(&draw_buf_, lv_buf1, lv_buf2, buf_size);

   //// lv_disp_drv_init(&disp_drv_);
   //// disp_drv_.hor_res = w;
   //// disp_drv_.ver_res = h;
   //// disp_drv_.draw_buf = &draw_buf_;
    ////disp_drv_.flush_cb = [](lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
  ////      auto *self = static_cast<M5PaperS3DisplayM5GFX *>(drv->user_data);
  ////      self->lvgl_flush(area, color_p);
 ////   };
   //// disp_drv_.user_data = this;
   //// lv_disp_t *disp = lv_disp_drv_register(&disp_drv_);
 ////   disp->driver.user_data = this;   // <-- add this

////    lv_disp_drv_register(&disp_drv_);

    // Optional LVGL label to test
 ////   lv_obj_t *label = lv_label_create(lv_scr_act());
 ////   lv_label_set_text(label, "Hello LVGL");
 ////   lv_obj_center(label);

    ESP_LOGD(TAG, "LVGL setup complete.");
}

// ... (update() method remains largely the same)
void M5PaperS3DisplayM5GFX::update() {
    static bool first_time = true;
    if (first_time) {
        ESP_LOGD(TAG, "Delaying first update for EPD...");
        delay(1000); // Allow EPD to settle, also good if setup had issues
        first_time = false;

        // Initial clear/refresh sequence (optional, but can help with ghosting from boot)
        ESP_LOGD(TAG, "Performing initial EPD clear sequence...");
        M5.Display.setEpdMode(epd_mode_t::epd_quality); // Ensure quality mode for full clear
        ESP_LOGD(TAG, "Initial EPD clear sequence finished.");
        //M5.Display.setEpdMode(epd_mode_t::epd_quality); // Back to desired mode
    }

    // Let LVGL render
    lv_timer_handler();

    // Actual EPD refresh happens automatically after pushImageGray()
    // but the M5PaperS3 DCS sequence requires display() sometimes:
    M5.Display.display(); 
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
    M5.update(); // Update touch and other inputs
    static unsigned long last_touch_time = 0;
unsigned long current_time = millis();

}

void M5PaperS3DisplayM5GFX::lvgl_flush(const lv_area_t *area, lv_color_t *color_p) {
   static uint32_t last_refresh = 0;
uint32_t now = millis();

if (now - last_refresh < 1000) {   // at least 120ms between partial updates
    lv_disp_flush_ready(&this->disp_drv_);
    return;
}

last_refresh = now;

    if (!area || !color_p) {
        lv_disp_flush_ready(&this->disp_drv_);
        return;
    }

    int x1 = area->x1;
    int y1 = area->y1;
    int x2 = area->x2;
    int y2 = area->y2;

    int w = x2 - x1 + 1;
    int h = y2 - y1 + 1;

    if (w <= 0 || h <= 0) {
        lv_disp_flush_ready(&this->disp_drv_);
        return;
    }

    // 2 line buffers (each w bytes)
    uint8_t *bufA = (uint8_t*) heap_caps_malloc(w, MALLOC_CAP_SPIRAM);
    uint8_t *bufB = (uint8_t*) heap_caps_malloc(w, MALLOC_CAP_SPIRAM);

    if (!bufA || !bufB) {
        ESP_LOGE("lvgl", "Failed to allocate PSRAM line buffers for flush.");
        if (bufA) heap_caps_free(bufA);
        if (bufB) heap_caps_free(bufB);
        lv_disp_flush_ready(&this->disp_drv_);
        return;
    }

    uint8_t *cur = bufA;
    uint8_t *next = bufB;

    lv_color_t *src_line = color_p;

    M5.Display.startWrite();

    for (int yy = 0; yy < h; yy++) {
        // Prepare next line in 'next'
        for (int xx = 0; xx < w; xx++) {
            next[xx] = rgb565_to_gray(src_line[xx].full);
        }

        // Push the "current" line
        if (yy > 0) {
            M5.Display.pushImageGray(x1, y1 + yy - 1, w, 1, cur);
        }

        // Swap buffers
        uint8_t *tmp = cur;
        cur = next;
        next = tmp;

        src_line += w;
    }

    // Push last line
    M5.Display.pushImageGray(x1, y1 + h - 1, w, 1, cur);

    M5.Display.endWrite();

    heap_caps_free(bufA);
    heap_caps_free(bufB);

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
