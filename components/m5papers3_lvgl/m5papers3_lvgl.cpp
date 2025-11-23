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

// ... (M5PaperS3DisplayM5GFX::setup() remains largely the same, ensure logging is as you need it)
void M5PaperS3DisplayM5GFX::setup() {
    ESP_LOGD(TAG, "Setting up M5PaperS3 Display...");

    // --- M5 Display init ---
    auto cfg = M5.config();
    M5.begin(cfg);
    vTaskDelay(pdMS_TO_TICKS(100));
    M5.Display.setEpdMode(epd_mode_t::epd_quality);
    vTaskDelay(pdMS_TO_TICKS(1000));

    // --- LVGL init ---
    lv_init();

    int w = this->get_width();
    int h = this->get_height();

    size_t buf_size = w * LV_BUF_LINES;

    // Allocate LVGL double buffers
    lv_color_t *lv_buf1 = (lv_color_t*)malloc(buf_size * sizeof(lv_color_t));
    lv_color_t *lv_buf2 = (lv_color_t*)malloc(buf_size * sizeof(lv_color_t));
    if (!lv_buf1 || !lv_buf2) {
        ESP_LOGE(TAG, "Failed to allocate LVGL buffers");
        return;
    }

    lv_disp_draw_buf_init(&draw_buf_, lv_buf1, lv_buf2, buf_size);
    lv_disp_drv_init(&disp_drv_);
    disp_drv_.hor_res = w;
    disp_drv_.ver_res = h;
    disp_drv_.draw_buf = &draw_buf_;
    disp_drv_.flush_cb = [](lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
        auto *self = static_cast<M5PaperS3DisplayM5GFX *>(drv->user_data);
        self->lvgl_flush(area, color_p);
    };
    disp_drv_.user_data = this;
    lv_disp_drv_register(&disp_drv_);

    // Optional test: simple LVGL label
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello LVGL");
    lv_obj_center(label);

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

    if (this->canvas_ == nullptr) {
        ESP_LOGE(TAG, "Canvas not available in update()!");
        return;
    }

    if (this->writer_ != nullptr) {
        ESP_LOGD(TAG, "Clearing canvas sprite (fill with white)");
        // Assuming palette index 15 is white for 4-bit grayscale.
        // Better to use this->gfx_.color565(255,255,255) or equivalent if palette changes.
       // this->canvas_->fillSprite(this->gfx_.color565(255, 255, 255));
        ESP_LOGD(TAG, "Calling writer lambda...");
      //  this->writer_(*this); // This is where user draws to the display (this->canvas_)
     ;
        ESP_LOGD(TAG, "Pushing sprite to display buffer (M5.Display)...");
        // The canvas (sprite) content is pushed to the actual physical display driver (M5.Display)
        
   //     this->canvas_->pushSprite(0, 0);
        ESP_LOGD(TAG, "Triggering EPD refresh (M5.Display.display())...");
        M5.Display.display(); // Tell the EPD to show what's in its buffer
    } else {
        ESP_LOGD(TAG, "No writer lambda set, skipping drawing. Pushing current canvas content.");
        // If no writer, we might still want to push the (potentially empty or old) canvas
        // and refresh the display, or do nothing.
        // For now, let's assume if no writer, no explicit update is needed beyond initial clear.
        // However, if there was a partial update, we might want to refresh.
        // M5.Display.display(); // Uncomment if you want to refresh even without a writer
    }

    ESP_LOGD(TAG, "EPD refresh process initiated."); // display() is often non-blocking for EPD
    
  // Call LVGL tasks
  lv_timer_handler();
}
//#include 
void M5PaperS3DisplayM5GFX::partial_update(int x, int y, int w, int h) {
  if (!canvas_) return;

  // Maak een tijdelijke sprite (overlay), die data kopieert uit canvas_
  lgfx::v1::LGFX_Sprite temp(&gfx_);
  temp.setColorDepth(4);
  temp.setPsram(true);
  temp.setPaletteGrayscale();
  temp.createSprite(w, h);

  // Kopieer pixel voor pixel uit canvas_ naar de temp sprite
  for (int dy = 0; dy < h; ++dy) {
    for (int dx = 0; dx < w; ++dx) {
      auto col = canvas_->readPixel(x + dx, y + dy);
      temp.drawPixel(dx, dy, col);
    }
  }

  // Push de temp sprite naar het e-paper scherm op juiste locatie
  temp.pushSprite(x, y);
}

void M5PaperS3DisplayM5GFX::dump_config() {
    LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper (4-bit Grayscale)", this);
    int display_rotation_deg = 0;
    // this->rotation_ stores M5GFX 0-3. Convert back for logging.
    if (this->rotation_ == 1) display_rotation_deg = 90;
    else if (this->rotation_ == 2) display_rotation_deg = 180;
    else if (this->rotation_ == 3) display_rotation_deg = 270;
    ESP_LOGCONFIG(TAG, "  Rotation (applied to M5.Display): %d degrees (M5GFX value %d)", display_rotation_deg, this->rotation_);

    if (this->canvas_) {
        ESP_LOGCONFIG(TAG, "  Canvas Size: %d x %d (gfx: %dx%d)", this->canvas_->width(), this->canvas_->height(), this->gfx_.width(), this->gfx_.height());
        ESP_LOGCONFIG(TAG, "  Canvas Color Depth: %d bits", this->canvas_->getColorDepth());
    } else {
        ESP_LOGCONFIG(TAG, "  Canvas: Not Initialized (gfx: %dx%d)", this->gfx_.width(), this->gfx_.height());
    }
}

M5PaperS3DisplayM5GFX::~M5PaperS3DisplayM5GFX() {
    if (this->canvas_ != nullptr) {
        delete this->canvas_;
        this->canvas_ = nullptr;
        ESP_LOGD(TAG, "Canvas deleted safely in destructor.");
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


int M5PaperS3DisplayM5GFX::get_width_internal() {
     return (this->canvas_) ? this->canvas_->width() : this->gfx_.width();
}

int M5PaperS3DisplayM5GFX::get_height_internal() {
    return (this->canvas_) ? this->canvas_->height() : this->gfx_.height();
}

void M5PaperS3DisplayM5GFX::fill(Color color) {
    if (this->canvas_ == nullptr) return;
    uint32_t rgb888_color = this->gfx_.color888(color.r, color.g, color.b);
    ESP_LOGV(TAG, "fill() called with RGB888: %x", rgb888_color);
    this->canvas_->fillSprite(rgb888_color);
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
    if (this->canvas_ == nullptr) return;
    // Bounds check should use canvas dimensions
    if (x < 0 || x >= this->canvas_->width() || y < 0 || y >= this->canvas_->height()) {
        return;
    }
    uint32_t rgb888_color = this->gfx_.color888(color.r, color.g, color.b);
    this->canvas_->drawPixel(x, y, rgb888_color);
}
void M5PaperS3DisplayM5GFX::loop() {
    M5.update(); // Update touch and other inputs
    static unsigned long last_touch_time = 0;
unsigned long current_time = millis();

}

void M5PaperS3DisplayM5GFX::set_writer(std::function<void(esphome::display::Display &)> writer) {
    this->writer_ = writer;
}

void M5PaperS3DisplayM5GFX::lvgl_flush(const lv_area_t *area, lv_color_t *color_p) {
  if (!area || !color_p) {
    lv_disp_flush_ready(&this->disp_drv_);
    return;
  }

  int32_t x1 = area->x1 < 0 ? 0 : area->x1;
  int32_t y1 = area->y1 < 0 ? 0 : area->y1;
  int32_t x2 = area->x2 >= (int)this->get_width() ? this->get_width() - 1 : area->x2;
  int32_t y2 = area->y2 >= (int)this->get_height() ? this->get_height() - 1 : area->y2;

  int w = x2 - x1 + 1;
  int h = y2 - y1 + 1;
  if (w <= 0 || h <= 0) {
    lv_disp_flush_ready(&this->disp_drv_);
    return;
  }

  lv_color_t *p = color_p;

  // Start SPI batch write to M5.Display
  M5.Display.startWrite();

  for (int yy = 0; yy < h; yy++) {
    for (int xx = 0; xx < w; xx++) {
      uint16_t c565 = p->full;

      // Convert RGB565 -> 8-bit grayscale
      uint8_t r5 = (c565 >> 11) & 0x1F;
      uint8_t g6 = (c565 >> 5) & 0x3F;
      uint8_t b5 = c565 & 0x1F;
      uint8_t r8 = (r5 * 527 + 23) >> 6;
      uint8_t g8 = (g6 * 259 + 33) >> 6;
      uint8_t b8 = (b5 * 527 + 23) >> 6;
      uint8_t lum = (uint8_t)((299 * r8 + 587 * g8 + 114 * b8) / 1000);

      // Map 0..255 -> 0..15 (4-bit grayscale)
      uint8_t gray4 = (lum * 15 + 127) / 255;

      // Convert 4-bit grayscale to RGB565 for display
      uint8_t gray8 = (gray4 * 255) / 15;
      uint16_t gray565 = ((gray8 >> 3) << 11) | ((gray8 >> 2) << 5) | (gray8 >> 3);

      M5.Display.drawPixel(x1 + xx, y1 + yy, gray565);

      p++;
    }
  }

  M5.Display.endWrite();

  // Tell LVGL we are done
  lv_disp_flush_ready(&this->disp_drv_);
}



} // namespace m5papers3_display_m5gfx
} // namespace esphome
