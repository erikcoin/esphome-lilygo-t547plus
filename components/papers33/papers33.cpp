#include "papers33.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "esphome/core/color.h"
#include <cmath>
#include <algorithm> // For std::min and std::max
#include <esp_heap_caps.h> // Include for heap info functions

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

// Helper function to convert ESPHome Color to a 4-bit grayscale palette index (0-15)
static inline uint8_t get_grayscale_palette_index(esphome::Color color) {
    float gray_value = (color.r + color.g + color.b) / 3.0f;
    float index_float = (255.0f - gray_value) / 255.0f * 15.0f;
    uint8_t index = static_cast<uint8_t>(roundf(index_float));
    index = std::min((uint8_t)15, std::max((uint8_t)0, index));
    return index;
}


void M5PaperS3DisplayM5GFX::setup() {
    ESP_LOGD(TAG, "Memory before M5.begin():");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Total PSRAM: %u bytes", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));


    ESP_LOGD(TAG, "Calling M5.config()...");
    auto cfg = M5.config();
   // //cfg.use_psram = true; // Ensure this is true
   // ESP_LOGD(TAG, "M5.config().use_psram set to: %s", cfg.use_psram ? "true" : "false");
    ESP_LOGD(TAG, "Calling M5.begin()...");

    M5.begin(cfg); // This is where I2C errors are seen in logs
    ESP_LOGD(TAG, "M5.begin() finished.");

    // --- Add a delay immediately after M5.begin() ---
    ESP_LOGD(TAG, "Adding delay after M5.begin()...");
    delay(100); // Small delay after M5.begin()
    ESP_LOGD(TAG, "Delay finished.");
    // --------------------------------------------------


    ESP_LOGD(TAG, "Memory after M5.begin() + delay:");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));


    ESP_LOGD(TAG, "Calling M5.Display.setEpdMode()...");
    M5.Display.setEpdMode(epd_mode_t::epd_quality);
    ESP_LOGD(TAG, "M5.Display.setEpdMode() finished.");

    ESP_LOGD(TAG, "Waiting for EPD to be readable...");
    while (!M5.Display.isReadable()) {
        ESP_LOGD(TAG, "EPD not readable yet, waiting...");
        delay(1000);
    }
    ESP_LOGD(TAG, "EPD is readable.");

    ESP_LOGD(TAG, "Adding delay after readable before touch/sprite creation...");
    delay(500);
    ESP_LOGD(TAG, "Delay finished.");

    // Touch block commented out as per user's testing
    /*
    if (this->touch_coordinates_sensor_ != nullptr) {
        ESP_LOGD(TAG, "Touch sensor configured, entering touch block...");
        ESP_LOGD(TAG, "Publishing test value to touch sensor...");
        this->touch_coordinates_sensor_->publish_state("42,84");
        ESP_LOGD(TAG, "Test value published.");
        ESP_LOGD(TAG, "Exiting touch block.");
    } else {
        ESP_LOGD(TAG, "Touch sensor not configured, skipping touch block.");
    }
    */

    auto &gfx = M5.Display;
    // Rotation code removed by user


    if (this->canvas_ != nullptr) {
      ESP_LOGD(TAG, "Deleting existing canvas_...");
      delete this->canvas_;
      this->canvas_ = nullptr;
      ESP_LOGD(TAG, "Existing canvas_ deleted.");
    }

    // --- Add a delay specifically before getting display dimensions ---
    ESP_LOGD(TAG, "Adding delay before getting display dimensions...");
    delay(100); // Delay before calling width()/height()
    ESP_LOGD(TAG, "Delay finished.");
    // -----------------------------------------------------------------

    // Get display dimensions using gfx
    // *** CRASH LIKELY HAPPENS HERE WHEN CALLING gfx.width() or gfx.height() ***
    ESP_LOGD(TAG, "About to call gfx.width() and gfx.height()..."); // New log before the call
    int display_width = gfx.width(); // Get dimensions BEFORE allocation test
    int display_height = gfx.height(); // Get dimensions BEFORE allocation test
    ESP_LOGD(TAG, "Finished calling gfx.width() and gfx.height(). Display dimensions: %d x %d", display_width, display_height); // Log after and get values again


    // Calculate required memory for the sprite buffer (based on retrieved dimensions)
    size_t required_bytes = (size_t)display_width * display_height * 4 / 8; // Use retrieved dimensions
    ESP_LOGD(TAG, "Estimated memory needed for sprite buffer (4 bit): %u bytes", required_bytes);


     // Log memory status before new LGFX_Sprite
    ESP_LOGD(TAG, "Memory before new LGFX_Sprite:");
    ESP_LOGD(TAG, "  Free Internal: %u bytes", heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGD(TAG, "  Largest Internal Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));

    ESP_LOGD(TAG, "Calling new lgfx::v1::LGFX_Sprite(&gfx)...");
    this->canvas_ = new lgfx::v1::LGFX_Sprite(&gfx);
    ESP_LOGD(TAG, "new lgfx::v1::LGFX_Sprite() finished.");
    this->canvas_->setPsram(true);  // <-- DIT IS BELANGRIJK

    if (this->canvas_ == nullptr) {
         ESP_LOGE(TAG, "Failed to create LGFX_Sprite object itself (out of internal RAM?)!");
         return;
    }

    ESP_LOGD(TAG, "Setting canvas color depth to 4 bits...");
    this->canvas_->setColorDepth(4);
    ESP_LOGD(TAG, "Canvas color depth set.");

    ESP_LOGD(TAG, "Setting canvas palette to grayscale...");
    this->canvas_->setPaletteGrayscale();
    ESP_LOGD(TAG, "Canvas palette set.");

     // Log memory status before createSprite (actual allocation)
    ESP_LOGD(TAG, "Memory before canvas_->createSprite():");
    ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    ESP_LOGD(TAG, "Calling canvas_->createSprite(%d, %d)...", display_width, display_height); // Log dimensions used

    // This is the call that allocates the main pixel buffer
    bool ok = this->canvas_->createSprite(display_width, display_height); // Use stored dimensions
    ESP_LOGD(TAG, "canvas_->createSprite() finished. Result: %s", ok ? "true" : "false");


    if (!ok) {
        ESP_LOGE(TAG, "Failed to create canvas sprite buffer! Check memory (PSRAM) and fragmentation.");
        ESP_LOGE(TAG, "Sprite allocation requested size: %d x %d", display_width, display_height); // Log dimensions requested
        // Log memory status after failed createSprite
        ESP_LOGE(TAG, "Memory after *failed* createSprite:");
        ESP_LOGE(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGE(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

        // Cleanup the sprite object itself if buffer allocation failed
        delete this->canvas_;
        this->canvas_ = nullptr;
    } else {
        ESP_LOGD(TAG, "Canvas sprite buffer created successfully.");
        ESP_LOGD(TAG, "Sprite allocated with size: %d x %d", this->canvas_->width(), this->canvas_->height());
        // Log memory status after successful createSprite
        ESP_LOGD(TAG, "Memory after *successful* createSprite:");
        ESP_LOGD(TAG, "  Free PSRAM: %u bytes", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
        ESP_LOGD(TAG, "  Largest PSRAM Free Block: %u bytes", heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));

        ESP_LOGD(TAG, "Calling canvas_->fillSprite()...");
        this->canvas_->fillSprite(0);
        ESP_LOGD(TAG, "canvas_->fillSprite() finished.");
    }

    if (!M5.Touch.isEnabled()) {
        ESP_LOGW(TAG, "Touchscreen not enabled or GT911 not found.");
    } else {
        ESP_LOGI(TAG, "Touchscreen initialized.");
    }
     ESP_LOGD(TAG, "End of setup().");

}

// ... rest of the .cpp file is the same ...
