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
    ESP_LOGD(TAG, "Memory before M5.begin():");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    ESP_LOGD(TAG, "Calling M5.config()...");
    auto cfg = M5.config();
    
    ESP_LOGD(TAG, "Calling M5.begin()...");
    M5.begin(cfg);
    ESP_LOGD(TAG, "M5.begin() finished.");

    ESP_LOGD(TAG, "Adding delay after M5.begin()...");
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGD(TAG, "Delay finished.");

    ESP_LOGD(TAG, "Memory after M5.begin() + delay:");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));

    ESP_LOGD(TAG, "Calling M5.Display.setEpdMode()...");
    M5.Display.setEpdMode(epd_mode_t::epd_quality);
    ESP_LOGD(TAG, "M5.Display.setEpdMode() finished.");

    ESP_LOGD(TAG, "Waiting for EPD to be readable...");
    // while (!M5.Display.isReadable()) { // This can hang if there's an issue
    //     ESP_LOGD(TAG, "EPD not readable yet, waiting...");
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }
    // ESP_LOGD(TAG, "EPD is readable.");
    // Consider a timeout or alternative check if isReadable() causes issues
    vTaskDelay(pdMS_TO_TICKS(1000)); // Give some time

    ESP_LOGD(TAG, "Adding delay after readable before touch/sprite creation...");
    //delay(500);
    ESP_LOGD(TAG, "Delay finished.");


ESP_LOGD(TAG, "voor lvgl setup");
  // === LVGL setup ===
  lv_init();
ESP_LOGD(TAG, "na lvgl setup");
  int w = this->get_width();
  int h = this->get_height();
  size_t buf_size = w * LV_BUF_LINES;

  // allocate two buffers (double buffering)
  lv_buf1_ = (lv_color_t*)malloc(buf_size * sizeof(lv_color_t));
  lv_buf2_ = (lv_color_t*)malloc(buf_size * sizeof(lv_color_t));
  if (!lv_buf1_ || !lv_buf2_) {
    ESP_LOGE("papers33", "Failed to allocate LVGL buffers");
  }

  lv_disp_draw_buf_init(&draw_buf_, lv_buf1_, lv_buf2_, buf_size);
ESP_LOGD(TAG, "voor drvinit ");
  lv_disp_drv_init(&disp_drv_);
  disp_drv_.hor_res = w;
  disp_drv_.ver_res = h;
  disp_drv_.draw_buf = &draw_buf_;
  disp_drv_.flush_cb = [](lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
    auto *self = static_cast<M5PaperS3DisplayM5GFX *>(drv->user_data);
    self->lvgl_flush(area, color_p);
  };
  disp_drv_.user_data = this;
ESP_LOGD(TAG, "na drvinit ");
  lv_disp_drv_register(&disp_drv_);
ESP_LOGD(TAG, "na  drvregister ");
  // Optionally: create a simple LVGL UI element to test
  lv_obj_t *label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "Hello LVGL");
  lv_obj_center(label);


    
    auto &gfx = M5.Display;
    if (this->canvas_ != nullptr) {
        ESP_LOGD(TAG, "Deleting existing canvas_...");
        delete this->canvas_;
        this->canvas_ = nullptr;
        ESP_LOGD(TAG, "Existing canvas_ deleted.");
    }

    ESP_LOGD(TAG, "About to call gfx.width() and gfx.height()...");
    int display_width = gfx.width();
    int display_height = gfx.height();
    ESP_LOGD(TAG, "Display dimensions: %d x %d (rotation %d applied to gfx by LovyanGFX)", display_width, display_height, gfx.getRotation());
    size_t required_bytes = (size_t)display_width * display_height * 4 / 8;
    ESP_LOGD(TAG, "Estimated memory needed for sprite buffer (4 bit): %u bytes", required_bytes);
    ESP_LOGD(TAG, "Memory before new LGFX_Sprite:");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "Calling new lgfx::v1::LGFX_Sprite(&gfx)...");
    this->canvas_ = new lgfx::v1::LGFX_Sprite(&gfx); // Sprite for the main display
    if (this->canvas_ == nullptr) {
        ESP_LOGE(TAG, "Failed to create LGFX_Sprite object itself (out of internal RAM?)!");
        return; // Cannot proceed
    }
    this->canvas_->setPsram(true);
    this->canvas_->setColorDepth(4);
    this->canvas_->setRotation(0); 
    this->canvas_->setPaletteGrayscale();

    ESP_LOGD(TAG, "Memory before canvas_->createSprite():");
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "Calling canvas_->createSprite(%d, %d)...", display_width, display_height);

    bool ok = this->canvas_->createSprite(display_width, display_height);
    ESP_LOGD(TAG, "canvas_->createSprite() finished. Result: %s", ok ? "true" : "false");
    ESP_LOGD(TAG, "Canvas buffer address: %p", this->canvas_->getBuffer());

    if (!ok) {
        ESP_LOGE(TAG, "Failed to create canvas sprite buffer! Check memory (PSRAM) and fragmentation.");
        ESP_LOGE(TAG, "Sprite allocation requested size: %d x %d", display_width, display_height);
        ESP_LOGE(TAG, "Memory after *failed* createSprite:");
        ESP_LOGE(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGE(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
        delete this->canvas_;
        this->canvas_ = nullptr;
    } else {
        ESP_LOGD(TAG, "Canvas sprite buffer created successfully. Size: %d x %d", this->canvas_->width(), this->canvas_->height());
        ESP_LOGD(TAG, "Canvas sprite buffer created successfully. colordepth: %d rotation: %d", this->canvas_->getColorDepth(), this->canvas_->getRotation());
int raw_depth = this->canvas_->getColorDepth();
int clean_depth = raw_depth & 0x0F;
ESP_LOGD(TAG, "Canvas color depth: %d (raw: 0x%X)", clean_depth, raw_depth);
        ESP_LOGD(TAG, "Memory after *successful* createSprite:");
        ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
        // this->canvas_->fillSprite(15); // Fill white initially
    }

    ESP_LOGD(TAG, "End of setup().");
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
        this->canvas_->fillSprite(this->gfx_.color565(255, 255, 255));
        ESP_LOGD(TAG, "Calling writer lambda...");
        this->writer_(*this); // This is where user draws to the display (this->canvas_)
     ;
        ESP_LOGD(TAG, "Pushing sprite to display buffer (M5.Display)...");
        // The canvas (sprite) content is pushed to the actual physical display driver (M5.Display)
        
        this->canvas_->pushSprite(0, 0);
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
  int32_t x1 = area->x1;
  int32_t y1 = area->y1;
  int32_t x2 = area->x2;
  int32_t y2 = area->y2;
  int w = x2 - x1 + 1;
  int h = y2 - y1 + 1;

  for (int yy = 0; yy < h; yy++) {
    for (int xx = 0; xx < w; xx++) {
      lv_color_t c = *color_p++;
      // Convert LVGL color to your e-paper color:
      // Assume LV_COLOR_DEPTH = 16 (RGB565)
      uint16_t c565 = c.full;
      // Convert to grayscale / your format — this depends on how `papers33` draws pixels
      // Example: map to 16 gray levels if epaper is 4-bit:
      uint8_t gray = (( (c565 >> 11) & 0x1F) * 299 + ((c565 >> 5) & 0x3F) * 587 + (c565 & 0x1F) * 114) / (1000 * 8); 
      // (this is an example — tune to your conversion)
      uint16_t final_color = gray; // or map to your color type

      // Use your drawing function
        esphome::Color d(final_color, final_color, final_color);   // grayscale
        this->draw_pixel_at(x1 + xx, y1 + yy, d);
    // this->draw_pixel_at(x1 + xx, y1 + yy, final_color);
    }
  }

  // After writing: flush to EPD
  //this->M5.Display.display(); // or whatever your method is to update the EPD
   //     this->M5.Display.display();
  // Let LVGL know we’re done
this->update();
    
    ESP_LOGD(TAG, "voor flushready ");
  lv_disp_flush_ready(&disp_drv_);
}

} // namespace m5papers3_display_m5gfx
} // namespace esphome
