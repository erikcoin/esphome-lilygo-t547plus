#include "m5papers3_display.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

void M5PaperS3DisplayM5GFX::setup() {
    ESP_LOGD(TAG, "Free heap: %d bytes", esp_get_free_heap_size());
    auto cfg = M5.config();
    M5.begin(cfg);
    //M5.Display.clearDisplay();
    ESP_LOGD(TAG, "M5.begin() finished.");
    M5.Display.setEpdMode(epd_mode_t::epd_fast);
    while (!M5.Display.isReadable()) {
        ESP_LOGD(TAG, "Waiting for EPD to be ready...");
        delay(1000);
    }
if (this->touch_coordinates_sensor != nullptr) {
        ESP_LOGD(TAG, "Publishing test value to touch sensor.");
        this->touch_coordinates_sensor->publish_state("42,84");
  }
    
   // M5.Display.clearDisplay();
    auto &gfx = M5.Display;
    gfx.setRotation(this->rotation_);
    ESP_LOGD(TAG, "M5GFX Rotation set to: %d", this->rotation_);
    //gfx.fillScreen(TFT_WHITE);
    

    if (this->canvas_ != nullptr) {
      delete this->canvas_;
    }
    this->canvas_ = new lgfx::LGFX_Sprite(&gfx);
    this->canvas_->setColorDepth(1);
    this->canvas_->setPaletteColor(0, TFT_WHITE);  // pixel value 0 = wit
    this->canvas_->setPaletteColor(1, TFT_BLACK);  // pixel value 1 = zwart
    bool ok = this->canvas_->createSprite(gfx.width(), gfx.height());
    if (!ok) {
        ESP_LOGE(TAG, "Failed to create canvas sprite!");
    } else {
        ESP_LOGD(TAG, "Canvas created with size: %d x %d", gfx.width(), gfx.height());
    }

    if (!M5.Touch.isEnabled()) {
        ESP_LOGW(TAG, "Touchscreen not enabled or GT911 not found.");
    } else {
        ESP_LOGI(TAG, "Touchscreen initialized.");
    }
//gfx.display();
 //   gfx.waitDisplay();
}

void M5PaperS3DisplayM5GFX::update() {
    static bool first_time = true;
    if (first_time) {
        ESP_LOGD(TAG, "Delaying first update for EPD...");
        delay(1000);
        first_time = false;
    }

   // M5.Display.setEpdMode(epd_mode_t::epd_quality);
    if (this->writer_ != nullptr) {
        ESP_LOGD(TAG, "Maak wit...");
        this->canvas_->fillSprite(TFT_WHITE);
        ESP_LOGD(TAG, "Start writer...");
        this->writer_(*this);
        //M5.Display.fillScreen(TFT_WHITE);  // indien gewenst
        ESP_LOGD(TAG, "pushsprite, pushing canvas...");
        this->canvas_->pushSprite(0, 0);
        ESP_LOGD(TAG, "Display...");
        this->gfx.display();  // Zorgt voor fysieke refresh van het e-paper scherm
    }

    update_touch();
}

// ======= Touch gerelateerde functies =======

bool M5PaperS3DisplayM5GFX::get_touch(TouchPoint *point) {
    m5::touch_point_t tp[1];  // Gebruik m5::touch_point_t
    int touch = M5.Display.getTouchRaw(tp, 1);
    if (touch > 0) {
        ESP_LOGD(TAG, "Raw touch detected at (%d, %d)", tp[0].x, tp[0].y);
        point->x = tp[0].x;
        point->y = tp[0].y;
        return true;
    }
   // ESP_LOGD(TAG, "No raw touch detected.");
    return false;
}

void M5PaperS3DisplayM5GFX::handle_touch(uint16_t x, uint16_t y) {
    ESP_LOGI(TAG, "Handling touch at (%d, %d)", x, y);
    M5.Lcd.fillCircle(x, y, 10, GREEN);
}

// ===========================================

void M5PaperS3DisplayM5GFX::dump_config() {
    LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper", this);
    ESP_LOGCONFIG(TAG, "  Rotation: %d degrees", this->rotation_ * 90);
}

M5PaperS3DisplayM5GFX::~M5PaperS3DisplayM5GFX() {
  if (this->canvas_ != nullptr) {
    delete this->canvas_;
    this->canvas_ = nullptr;
    ESP_LOGD(TAG, "Canvas deleted safely.");
  }
}

void M5PaperS3DisplayM5GFX::set_rotation(int rotation) {
    int m5gfx_rotation = 0;
    switch (rotation) {
        case 90: m5gfx_rotation = 1; break;
        case 180: m5gfx_rotation = 2; break;
        case 270: m5gfx_rotation = 3; break;
        default: m5gfx_rotation = 0; break;
    }
    this->rotation_ = m5gfx_rotation;
}

int M5PaperS3DisplayM5GFX::get_width_internal() {
    return M5.Display.width();
}

int M5PaperS3DisplayM5GFX::get_height_internal() {
    return M5.Display.height();
}

//void M5PaperS3DisplayM5GFX::fill(Color color) {
//    uint16_t col = color.is_on() ? TFT_BLACK : TFT_WHITE;
//    this->canvas_->fillSprite(col);
//}
void M5PaperS3DisplayM5GFX::fill(Color color) {
    uint16_t col = (color.raw_32 == 0xFFFFFFFF) ? TFT_WHITE : TFT_BLACK;
    this->canvas_->fillSprite(col);
}
void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
    uint16_t col = (color.raw_32 == 0xFFFFFFFF) ? TFT_WHITE : TFT_BLACK;
    this->canvas_->drawPixel(x, y, col);
}


//void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
//    uint16_t col = color.is_on() ? TFT_BLACK : TFT_WHITE;
//    this->canvas_->drawPixel(x, y, col);
//}

void M5PaperS3DisplayM5GFX::update_touch() {
  TouchPoint point;
  if (!this->get_touch(&point)) {
    return;
  }

  ESP_LOGD("m5papers3.display_m5gfx", "Raw touch detected at (%d, %d)", point.x, point.y);

  if (this->touch_coordinates_sensor == nullptr) {
    ESP_LOGW("m5papers3.display_m5gfx", "Touch coordinates sensor not initialized!");
    return;
  }

 this->send_coordinates(point);
    
}


void M5PaperS3DisplayM5GFX::set_touch_sensor(text_sensor::TextSensor *touch_coordinates_sensor) {
  ESP_LOGD(TAG, "Setting touch_coordinates_sensor...");
  this->touch_coordinates_sensor = touch_coordinates_sensor;
  ESP_LOGD(TAG, "Setting touch_coordinates_sensor is set");
  
    // Stel een interval in om elke 100 ms de update_touch() functie aan te roepen
  this->set_interval(100, [this]() { 
    this->update_touch();
  });

}


void M5PaperS3DisplayM5GFX::send_coordinates(TouchPoint tp) {
  if (this->touch_coordinates_sensor != nullptr) {

    std::string coords = std::to_string(tp.x) + "," + std::to_string(tp.y);
    this->touch_coordinates_sensor->publish_state(coords);
    ESP_LOGD("custom", "Sending coordinates: %s", coords.c_str());
    // Correct usage of set_timeout with capture
    App.scheduler.set_timeout(this, "clear_touch_sensor", 200, [this]() {
      if (this->touch_coordinates_sensor != nullptr)
        this->touch_coordinates_sensor->publish_state("");
    });
      
  } else {
    ESP_LOGW("custom", "Touch coordinates sensor not initialized!");
  }
}

void M5PaperS3DisplayM5GFX::set_writer(std::function<void(display::Display &)> writer) {
  this->writer_ = writer;
}
} // namespace m5papers3_display_m5gfx
} // namespace esphome
