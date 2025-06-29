#include "papers33.h"
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
    delay(500);
    ESP_LOGD(TAG, "Delay finished.");

    if (this->touch_coordinates_sensor_ != nullptr) {
        ESP_LOGD(TAG, "Touch sensor configured, publishing test value...");
        this->touch_coordinates_sensor_->publish_state("42,84"); // Indicate test
        ESP_LOGD(TAG, "Test value published.");
    } else {
        ESP_LOGD(TAG, "Touch sensor not configured.");
    }

    auto &gfx = M5.Display;
    // Apply initial rotation to M5.Display IF it's not done by LovyanGFX automatically
    // This->rotation_ is set via set_rotation which converts degrees to 0-3
    // M5.Display.setRotation(this->rotation_); // Typically LovyanGFX handles this based on its config or calls
    //this->on_press_triggers_[i] = Trigger<>();

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

    if (!M5.Touch.isEnabled()) {
        ESP_LOGW(TAG, "Touchscreen not enabled or GT911 not found.");
    } else {
        ESP_LOGI(TAG, "Touchscreen initialized.");
    }
    ESP_LOGD(TAG, "End of setup().");
}


// Method to add a button (called from generated code)
void M5PaperS3DisplayM5GFX::add_button(int x, int y, int width, int height, const std::string &buttonid, Trigger<> *trigger) {
    ESP_LOGD(TAG, "Adding button: x=%d, y=%d, w=%d, h=%d, has_action=%s",
             x, y, width, height, (trigger != nullptr ? "true" : "false"));
    ButtonConfig button_cfg;
    button_cfg.x = x;
    button_cfg.y = y;
    button_cfg.width = width;
    button_cfg.height = height;
    button_cfg.buttonid = buttonid;
    button_cfg.trigger = trigger;
    this->buttons_.push_back(button_cfg);

    ESP_LOGD(TAG, "Button trigger assigned? %s", (button_cfg.trigger != nullptr ? "Yes" : "No"));
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
        //draw_button(3,true);
        ESP_LOGD(TAG, "Pushing sprite to display buffer (M5.Display)...");
        // The canvas (sprite) content is pushed to the actual physical display driver (M5.Display)
        draw_all_buttons();
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
    
this->update_touch();
  

}


bool M5PaperS3DisplayM5GFX::get_touch(TouchPoint *point) {
    m5::touch_point_t tp[1]; // M5Unified uses m5::touch_point_t
    // M5.Display.getTouch uses LovyanGFX's touch system
    // M5.Touch.getDetail() uses M5Unified's direct touch controller interface
    
    // Let's try M5.Touch as it's more abstract from M5Unified
    if (M5.Touch.getCount() > 0) {
        auto touch_detail = M5.Touch.getDetail(); // Get primary touch point detail
        // Check if the point is valid (e.g., within screen bounds, if necessary)
        // The coordinates from M5.Touch should already be mapped to screen rotation
        point->x = touch_detail.x;
        point->y = touch_detail.y;
        ESP_LOGV(TAG, "Touch detected via M5.Touch: x=%d, y=%d, id=%d, size=%d", touch_detail.x, touch_detail.y, touch_detail.id, touch_detail.size);
        return true;
    }
    return false;
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

  // Sprite wordt automatisch gedealloceerd
}








void M5PaperS3DisplayM5GFX::update_touch() {
    TouchPoint tp;
    if (this->get_touch(&tp)) {
        ESP_LOGD(TAG, "Touch at x=%d, y=%d", tp.x, tp.y);
        if (this->touch_coordinates_sensor_ != nullptr) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%d,%d", tp.x, tp.y);
            this->touch_coordinates_sensor_->publish_state(buf);
        }
        this->send_coordinates_and_check_buttons(tp);
    }
}

void M5PaperS3DisplayM5GFX::send_coordinates_and_check_buttons(TouchPoint tp) {
static unsigned long last_touch_time = 0;
unsigned long current_time = millis();
if (current_time - last_touch_time < 1000) { // Ignore touches within 300ms
    return;
}
last_touch_time = current_time;
    for (auto &button : this->buttons_) {
        bool within_x = tp.x >= button.x && tp.x <= (button.x + button.width);
        bool within_y = tp.y >= button.y && tp.y <= (button.y + button.height);
        if (within_x && within_y) {
            ESP_LOGI(TAG, "Touch inside button area at (%d, %d, %d, %d)", button.x, button.y, button.width, button.height);
            // Toggle de status van de knop
            bool &state = this->button_states_[button.buttonid];
            state = !state;
            if (button.trigger != nullptr) {
                ESP_LOGI(TAG, "Triggering button action...");
                button.trigger->trigger();  // Trigger the action
                ESP_LOGI(TAG, "Trigger execution should be complete.");
            } else {
                ESP_LOGW(TAG, "Button has no trigger associated!");
            }
            return; // Optional: stop after first matching button
        }
    }
}



void M5PaperS3DisplayM5GFX::set_touch_sensor(text_sensor::TextSensor *touch_coordinates_sensor) {
    ESP_LOGD(TAG, "Setting touch_coordinates_sensor...");
    this->touch_coordinates_sensor_ = touch_coordinates_sensor;
    ESP_LOGD(TAG, "Touch_coordinates_sensor is set");
    // Poll for touch at a regular interval
    this->set_interval("touch_poll", 100, [this]() { this->update_touch(); });
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
    if (this->touch_coordinates_sensor_ != nullptr) {
        LOG_TEXT_SENSOR("  ", "Touch Coordinates Sensor", this->touch_coordinates_sensor_);
    }
    ESP_LOGCONFIG(TAG, "  Configured Buttons: %d", this->buttons_.size());
    for(const auto& btn : this->buttons_){
        ESP_LOGCONFIG(TAG, "    Button: x=%d, y=%d, w=%d, h=%d, action=%s", btn.x, btn.y, btn.width, btn.height, (btn.trigger ? "Yes" : "No"));
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
 //this->update_touch();
    

    M5.update(); // Update touch and other inputs
    static unsigned long last_touch_time = 0;
unsigned long current_time = millis();
if (current_time - last_touch_time < 1000) { // Ignore touches within 300ms
    return;
}
last_touch_time = current_time;
    TouchPoint tp;
   if (get_touch(&tp)) {  // Check if touch is detected
        ESP_LOGD(TAG, "Touch from loop detected at x=%d, y=%d", tp.x, tp.y);
        send_coordinates_and_check_buttons(tp); // Process button interactions
       delay(200);
    }

    //vTaskDelay(pdMS_TO_TICKS(300)); // Small delay to prevent excessive polling
}

void M5PaperS3DisplayM5GFX::set_writer(std::function<void(esphome::display::Display &)> writer) {
    this->writer_ = writer;
}
//Trigger<> *M5PaperS3DisplayM5GFX::make_button_trigger(const std::string &buttonid) {
//    auto trig = std::make_unique<Trigger<>>();
 //   Trigger<> *ptr = trig.get();
//    this->triggers_.push_back(std::move(trig));
 //   return ptr;
    
//}
Trigger<> *M5PaperS3DisplayM5GFX::make_button_trigger(const std::string &buttonid) {
  auto it = this->button_triggers_.find(buttonid);
  if (it != this->button_triggers_.end()) {
    return it->second.get();  // Trigger bestaat al
  }

  // Nieuwe trigger aanmaken en opslaan
  auto trig = std::make_unique<Trigger<>>();
  Trigger<> *ptr = trig.get();
  this->button_triggers_[buttonid] = std::move(trig);
  return ptr;
}

void M5PaperS3DisplayM5GFX::draw_button(int index) {
  if (index < 0 || index >= this->buttons_.size()) return;
  auto &btn = this->buttons_[index];

  int x = btn.x;
  int y = btn.y;
  int w = btn.width;
  int h = btn.height;

  uint16_t fill_color = btn.is_pressed ? 0x0000 : 0xFFFF;  // Donkerder als ingedrukt
  uint16_t border_color = 0x0000; // zwart

  // Teken de knop
  canvas_->fillRect(x, y, w, h, fill_color);
  canvas_->drawRect(x, y, w, h, border_color);

  // Eventuele knoptekst (optioneel)
  canvas_->setTextColor(btn.is_pressed ? 0xFFFF : 0x0000);
  canvas_->setTextSize(1);
  canvas_->setCursor(x + 10, y + h / 2 - 4);
  canvas_->print("Knop ");
  canvas_->print(index + 1);
}

void M5PaperS3DisplayM5GFX::draw_all_buttons() {
  for (int i = 0; i < this->buttons_.size(); i++) {
    draw_button(i);
  }
}

void M5PaperS3DisplayM5GFX::press_button_effect(int index, int duration_ms) {
  if (index < 0 || index >= this->buttons_.size()) return;

  this->buttons_[index].is_pressed = true;
  //draw_all_buttons();
    draw_button(i);
  canvas_->pushSprite(0, 0);

  set_timeout(duration_ms, [this, index]() {
    this->buttons_[index].is_pressed = false;
    //draw_all_buttons();
      draw_button(i);
    canvas_->pushSprite(0, 0);
  });
}

} // namespace m5papers3_display_m5gfx
} // namespace esphome
