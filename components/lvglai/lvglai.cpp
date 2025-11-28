#include "lvglai.h"

// include LovyanGFX + M5Unified (adjust if your project uses different header names)
#include <M5Unified.h>        // gives M5.Display and LovyanGFX (M5Unified)
//#include <lgfx/v1.h>         // LGFX Sprite type
#include <atomic>

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *TAG = "m5papers3.display_m5gfx";

M5PaperS3DisplayM5GFX::~M5PaperS3DisplayM5GFX() {
  // free canvas if created
  if (this->canvas_) {
    this->canvas_->deleteSprite();
    delete this->canvas_;
    this->canvas_ = nullptr;
  }
  // don't call M5.Display.display() here
}

void M5PaperS3DisplayM5GFX::setup() {
  ESP_LOGD(TAG, "M5PaperS3DisplayM5GFX::setup() start");

  // Initialize M5 hardware as you had before
  auto cfg = M5.config();
  M5.begin(cfg);
  vTaskDelay(pdMS_TO_TICKS(100));

  // create canvas now
  this->ensure_canvas_created();
  if (!this->canvas_) {
    ESP_LOGE(TAG, "Canvas creation failed, display will remain uninitialized");
    this->initialized_ = false;
    return;
  }

  // Fill canvas white initially (palette index 15 assumed white)
  this->canvas_->fillSprite(0x0F);

  // Mark initialized
  this->initialized_ = true;
  this->dirty_.store(true);  // initial full refresh
  ESP_LOGD(TAG, "Canvas created: %dx%d colorDepth=%d (PSRAM: %u free)",
           this->canvas_->width(), this->canvas_->height(), this->canvas_->getColorDepth(),
           heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
  // Enable touch
  if (M5.Touch.isEnabled()) {
    ESP_LOGI(TAG, "Touch controller (GT911) enabled");
  } else {
    ESP_LOGW(TAG, "Touch controller not detected");
  }
}

void M5PaperS3DisplayM5GFX::update() {
  // Called frequently by ESPHome loopTask. Keep it short.
  if (!this->initialized_) return;

  // If any pixels changed, flush the whole canvas to the display once.
  // This batches all pixel writes (from draw_pixel_at) into a single push.
  if (this->dirty_.exchange(false)) {
    // push sprite to screen
    flush_canvas_to_display();
  }
   // Poll touch here
  poll_touch();
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
  // Called by ESPHome/LVGL pixel renderer.
  if (!this->initialized_) {
    // ignore early drawing attempts during boot
    return;
  }

  // bounds check
  if (x < 0 || y < 0 || x >= this->get_width() || y >= this->get_height()) return;

  // make sure canvas exists
  if (!this->canvas_) {
    this->ensure_canvas_created();
    if (!this->canvas_) return;
  }

  // compute palette index 0..15
  uint8_t idx = color_to_gray4(color);

  // draw into sprite (fast, in RAM)
  // LGFX Sprite in 4-bit mode expects color index (0..15)
  this->canvas_->drawPixel(x, y, idx);

  // mark dirty (will be flushed next update())
  this->dirty_.store(true);
}

// convert esphome::Color to 4-bit grayscale palette index (0..15)
// esphome::Color has r,g,b 0..255 accessors available via .red(), .green(), .blue()
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

void M5PaperS3DisplayM5GFX::ensure_canvas_created() {
  if (this->canvas_) return;

  // create LGFX sprite in PSRAM
  auto &gfx = M5.Display; // reference to LovyanGFX device
  lgfx::v1::LGFX_Sprite *s = new lgfx::v1::LGFX_Sprite(&gfx);
  if (!s) {
    ESP_LOGE(TAG, "Failed to new LGFX_Sprite");
    return;
  }

  s->setPsram(true);           // allocate sprite buffer in PSRAM
  s->setColorDepth(4);         // 4-bit palette indices
  s->setRotation(0);
  s->setPaletteGrayscale();    // setup palette 0..15 => grayscale

  // Create sprite sized to panel width/height
  const int w = this->get_width();
  const int h = this->get_height();

  ESP_LOGD(TAG, "Creating sprite %d x %d in PSRAM...", w, h);
  bool ok = s->createSprite(w, h);
  if (!ok) {
    ESP_LOGE(TAG, "createSprite failed! Free PSRAM: %u", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    delete s;
    return;
  }

  // store pointer
  this->canvas_ = s;
  ESP_LOGD(TAG, "Sprite created: %d x %d colorDepth=%d buf=%p",
           this->canvas_->width(), this->canvas_->height(), this->canvas_->getColorDepth(), (void*) this->canvas_->getBuffer());
}

void M5PaperS3DisplayM5GFX::flush_canvas_to_display() {
  if (!this->canvas_) return;
  // push sprite content to M5.Display internal buffer
  // pushSprite copies sprite -> device framebuffer
  // It's implemented inside LovyanGFX and will do the correct conversion.
  ESP_LOGD(TAG, "Pushing sprite to display (pushSprite)...");
  this->canvas_->pushSprite(0, 0);

  // Then tell EPD to update to what's in its internal framebuffer
  ESP_LOGD(TAG, "Calling M5.Display.display() to refresh EPD...");
  M5.Display.display();
  ESP_LOGD(TAG, "Display refresh requested.");
}

void M5PaperS3DisplayM5GFX::poll_touch() {
  if (!M5.Touch.isEnabled()) return;

  // Check how many touch points are active
  uint8_t count = M5.Touch.getCount();
  if (count > 0) {
    // Get the first touch point
    auto p = M5.Touch.getDetail(0);

    ESP_LOGD(TAG, "Touch at (%d,%d) pressed=%d", p.x, p.y, p.isPressed());

    this->last_touch_x_ = p.x;
    this->last_touch_y_ = p.y;
    this->last_touch_pressed_ = p.isPressed();
  } else {
    this->last_touch_pressed_ = false;
  }
}
}  // namespace m5papers3_display_m5gfx
}  // namespace esphome
