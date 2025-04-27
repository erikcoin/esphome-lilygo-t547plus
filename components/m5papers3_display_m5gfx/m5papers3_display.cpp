#include "m5papers3_display.h" // Zorg dat de .h bestandsnaam klopt
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

void M5PaperS3DisplayM5GFX::setup() {
    // Controleer het vrije geheugen
    ESP_LOGD(TAG, "Free heap: %d bytes", esp_get_free_heap_size());

    auto cfg = M5.config();
    M5.begin(cfg);

    ESP_LOGD(TAG, "M5.begin() finished.");
    M5.Display.setEpdMode(epd_mode_t::epd_fastest);
    while (!M5.Display.isReadable()) {
        ESP_LOGD(TAG, "Waiting for EPD to be ready...");
        delay(1000);
    }

    M5.Display.clearDisplay();
    auto &gfx = M5.Display;
    gfx.setRotation(this->rotation_);
    ESP_LOGD(TAG, "M5GFX Rotation set to: %d", this->rotation_);
    
    gfx.fillScreen(TFT_WHITE);
    gfx.display();
    gfx.waitDisplay();

    // Setup canvas
    ESP_LOGD(TAG, "Creating canvas...");
    this->canvas_ = M5Canvas(&gfx);
    this->canvas_.setColorDepth(1);  // Grayscale: 8-bit is prima

    bool ok = this->canvas_.createSprite(gfx.width(), gfx.height());
    if (!ok) {
        ESP_LOGE(TAG, "Failed to create canvas sprite!");
        // Optioneel: probeer opnieuw of log het geheugen verder
    } else {
        ESP_LOGD(TAG, "Canvas created with size: %d x %d", gfx.width(), gfx.height());
    }
    //canvas_.fillSprite(WHITE);  // Fill white at start
}

void M5PaperS3DisplayM5GFX::update() {
    static bool first_time = true;
    if (first_time) {
        ESP_LOGD(TAG, "Delaying first update for EPD...");
        delay(1000);  // 👈 éénmalige vertraging bij de eerste update
        first_time = false;
    }
    ESP_LOGD(TAG, "Running M5GFX display update...");

    M5.Display.setEpdMode(epd_mode_t::epd_fastest);
    if (this->writer_ != nullptr) {
        ESP_LOGD(TAG, "Maak wit...");
       // this->canvas_.fillSprite(TFT_BLACK);
       // uint16_t col = color.is_on() ? 0x0000 : 0xFFFF;
        this->canvas_.fillSprite(TFT_WHITE);  // begin met wit scherm
        //this->canvas_.pushSprite(0, 0);
       // this->canvas_.setTextColor(TFT_BLACK);
        ESP_LOGD(TAG, "Start writer...");
        // Schrijf naar scherm met behulp van de lambda
       // this->writer_(*this);  
        ESP_LOGD(TAG, "Lambda writer done, pushing canvas...");
        //delay(5000);
        // Push canvas naar display
        this->do_update_();
        ESP_LOGD(TAG, "do_update  done, pushing canvas...");
        this->canvas_.pushSprite(0, 0);
        ESP_LOGD(TAG, "pushsprite  done, pushing canvas...");
       // delay(5000);
        // Forceer een volledige e-paper update
       // M5.Display.display();                 
      //  M5.Display.waitDisplay();
    }
}



void M5PaperS3DisplayM5GFX::dump_config() {
  LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper", this);
  ESP_LOGCONFIG(TAG, "  Rotation: %d degrees", this->rotation_ * 90);
  LOG_UPDATE_INTERVAL(this);
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, Color color) {
  // Roep gewoon de interne versie aan
  this->draw_absolute_pixel_internal(x, y, color);
}




// --- Display Overrides ---
// set_rotation() blijft zoals in de vorige correctie
void M5PaperS3DisplayM5GFX::set_rotation(int rotation) {
  int m5gfx_rotation = 0;
  switch (rotation) {
    case 90: m5gfx_rotation = 1; break;
    case 180: m5gfx_rotation = 2; break;
    case 270: m5gfx_rotation = 3; break;
    default: m5gfx_rotation = 0; break;
  }
  this->rotation_ = m5gfx_rotation;
  // Meteen toepassen als setup al geweest is? Optioneel.
  // if (this->is_ready()) { // is_ready() is wel een geldige check
  //   M5.Display.setRotation(this->rotation_);
  // }
}


// get_width/height_internal(): Verwijder de !this->is_setup_ check
int M5PaperS3DisplayM5GFX::get_width_internal() {
  // Vertrouw erop dat M5.Display geldig is wanneer dit wordt aangeroepen na setup()
  // Een check op M5.getBoard() is minder nuttig hier, M5.Display.width() is directer.
  return M5.Display.width();
}

int M5PaperS3DisplayM5GFX::get_height_internal() {
  return M5.Display.height();
}

// fill() blijft zoals in de vorige correctie
void M5PaperS3DisplayM5GFX::fill(Color color) {
    
    uint16_t col = esphome::color.is_on() ? 0x0000 : 0xFFFF;
    ESP_LOGD(TAG, "Canvas fill aangeroepen: %d",col);
 // uint32_t native_color = get_native_m5gfx_color_(color);
 //   uint8_t gray = get_native_m5gfx_color_(color);
  this->canvas_.fillSprite(col);
    //M5.Display.fillScreen(col);
}

// --- Protected Display Overrides ---
// draw_absolute_pixel_internal() blijft zoals in de vorige correctie
void M5PaperS3DisplayM5GFX::draw_absolute_pixel_internal(int x, int y, Color color) {
   if (x < 0 || x >= this->get_width_internal() || y < 0 || y >= this->get_height_internal())
    return;
 // uint8_t gray = this->get_native_m5gfx_color_(color);
 //   ESP_LOGD(TAG,"DE KLEUR IS:(%d)", gray);
//  this->canvas_.drawPixel(x, y, gray);
    uint16_t col = Color.is_on() ? TFT_BLACK : TFT_WHITE;
    
   //uint32_t native_color = get_native_m5gfx_color_(color);
 //  ESP_LOGD(TAG, "draw_pixel: (%d, %d, %d)", x, y,col);
//voor schrijven naar scherm doe deze:
 //   M5.Display.drawPixel(x, y, col);
 //voor schrijven naar canvas doe deze:
    canvas_.drawPixel(x, y, col);
}

// Zet esphome kleur om naar 4-bit grijswaarde (0-15)
//uint8_t M5PaperS3DisplayM5GFX::get_native_m5gfx_color_(Color color) {
//  // brightness() geeft float tussen 0.0 en 1.0
// // float brightness = color.brightness();
//  float brightness = (0.299f * color.r + 0.587f * color.g + 0.114f * color.b) / 255.0f;
//  uint8_t gray = static_cast<uint8_t>(brightness * 15.0f);  // voor 16 grijsniveaus
 // return gray;
//}
// --- Helper Functie ---
//uint32_t M5PaperS3DisplayM5GFX::get_native_m5gfx_color_(Color color) {
    // !! Gebruik color.r, color.g, color.b (uint8_t) en converteer naar float !!
//    float r_f = color.r / 255.0f;
//    float g_f = color.g / 255.0f;
//    float b_f = color.b / 255.0f;

    // Luminantie formule (gewogen gemiddelde)
//    float gray_f = (r_f * 0.2126f + g_f * 0.7152f + b_f * 0.0722f);
    // Schaal naar 0-255
//    uint8_t gray_8bit = static_cast<uint8_t>(gray_f * 255.0f);

    // Converteer 8-bit gray naar M5GFX kleur (RGB888 formaat)
    

//    ESP_LOGD(TAG, "Input Color: R=%u G=%u B=%u", color.r, color.g, color.b);
//    ESP_LOGD(TAG, "Float Color: R=%.3f G=%.3f B=%.3f", r_f, g_f, b_f);
//    ESP_LOGD(TAG, "Grayscale (float): %.3f", gray_f);
 //   ESP_LOGD(TAG, "Grayscale (8-bit): %u", gray_8bit);
//   // ESP_LOGD(TAG, "M5GFX RGB888 color: 0x%06X", native_color);
//    return M5.Display.color888(gray_8bit, gray_8bit, gray_8bit);
//}

} // namespace m5papers3_display_m5gfx
} // namespace esphome
