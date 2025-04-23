#include "m5papers3_display.h" // Zorg dat de .h bestandsnaam klopt
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace m5papers3_display_m5gfx {

static const char *const TAG = "m5papers3.display_m5gfx";

// --- Component Overrides ---
// setup() en dump_config() blijven zoals in de vorige correctie

void M5PaperS3DisplayM5GFX::setup() {
  ESP_LOGD(TAG, "Setting up M5Paper S3 display using M5GFX...");
  auto cfg = M5.config();
  
  M5.begin(cfg);
  //M5.begin();
ESP_LOGD(TAG, "M5.begin() finished.");
M5.Display.setEpdMode(epd_mode_t::epd_fastest);
while (!M5.Display.isReadable()) {
  ESP_LOGD(TAG, "Waiting for EPD to be ready...");
  delay(100);
}

  M5.Display.clearDisplay();
  auto &gfx = M5.Display;
  gfx.setRotation(this->rotation_);
  ESP_LOGD(TAG, "M5GFX Rotation set to: %d", this->rotation_);
  // Optioneel ook: setEpdMode al hier
  
  gfx.fillScreen(TFT_WHITE);
  gfx.display();    // Forceer een volledige witte refresh
  gfx.waitDisplay(); // 👈 wacht écht tot klaar
  
  //this->canvas_ = lgfx::LGFX_Sprite(&gfx);
 // int w = gfx.width();
  //int h = gfx.height();
  //this->canvas_.createSprite(gfx.width(), gfx.height());
  //ESP_LOGD(TAG, "M5GFX Sprite created with size: %d x %d", w, h);

  //this->canvas_.setColorDepth(8);
  //ESP_LOGD(TAG, "Sprite color depth set to %d", this->canvas_.getColorDepth());

  //uint32_t white_color = get_native_m5gfx_color_(Color::WHITE);
  //gfx.fillScreen(white_color);
  //this->canvas_.fillSprite(white_color);

  //M5.Display.fillScreen(M5.Display.color888(255,255,255));
  //M5.Display.drawPixel(20, 20, M5.Display.color888(0, 0, 0));
  //M5.Display.display();
  M5.Display.waitDisplay();
  
  ESP_LOGCONFIG(TAG, "M5Paper S3 M5GFX Display setup complete.");
}

void M5PaperS3DisplayM5GFX::dump_config() {
  LOG_DISPLAY("", "M5Paper S3 M5GFX E-Paper", this);
  ESP_LOGCONFIG(TAG, "  Rotation: %d degrees", this->rotation_ * 90);
  LOG_UPDATE_INTERVAL(this);
}

void M5PaperS3DisplayM5GFX::draw_pixel_at(int x, int y, esphome::Color color) {
  // Roep gewoon de interne versie aan
  this->draw_absolute_pixel_internal(x, y, color);
}

// --- PollingComponent / Display Override ---
// update() blijft zoals in de vorige correctie
void M5PaperS3DisplayM5GFX::update() {

    static bool first_time = true;
  if (first_time) {
    ESP_LOGD(TAG, "Delaying first update for EPD...");
    delay(1000);  // 👈 éénmalige delay bij eerste update
    first_time = false;
  }
  ESP_LOGD(TAG, "Running M5GFX display update...");
  M5.Display.startWrite();
////  M5.Display.clearDisplay();
  if (this->writer_ != nullptr) {
    ESP_LOGD(TAG, "Calling display lambda...");
    this->writer_(*this);
    ESP_LOGD(TAG, "Display lambda done.");
  }
  M5.Display.endWrite();
  M5.Display.display();
////  M5.Display.startWrite();
////  //M5.Display.fillScreen(TFT_BLACK);
////  M5.Display.fillRect(20, 100, 280, 60, LIGHTGREY);
////  M5.Display.endWrite();
////  M5.Display.startWrite();
  //M5.Display.fillScreen(TFT_WHITE);
////M5.Display.fillRect(20, 480, 280, 60, TFT_BLACK);
////  M5.Display.endWrite();
  //ESP_LOGD(TAG, "Pushing M5GFX sprite to display...");
  //this->canvas_.pushSprite(0, 0);

////  ESP_LOGD(TAG, "Calling M5.Display.display() to refresh EPD...");
////  M5.Display.setEpdMode(epd_mode_t::epd_fastest);  // expliciet modus
////  M5.Display.display();
////  M5.Display.waitDisplay();  // wacht op EPD-complete refresh
///  ESP_LOGD(TAG, "M5GFX display update finished.");
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
  uint32_t native_color = get_native_m5gfx_color_(color);
 // this->canvas_.fillSprite(native_color);
}

// --- Protected Display Overrides ---
// draw_absolute_pixel_internal() blijft zoals in de vorige correctie
void M5PaperS3DisplayM5GFX::draw_absolute_pixel_internal(int x, int y, Color color) {
   uint32_t native_color = get_native_m5gfx_color_(color);
 //  this->canvas_.drawPixel(x, y, native_color);
}

// --- Helper Functie ---
uint32_t M5PaperS3DisplayM5GFX::get_native_m5gfx_color_(Color color) {
    // !! Gebruik color.r, color.g, color.b (uint8_t) en converteer naar float !!
    float r_f = color.r / 255.0f;
    float g_f = color.g / 255.0f;
    float b_f = color.b / 255.0f;

    // Luminantie formule (gewogen gemiddelde)
    float gray_f = (r_f * 0.2126f + g_f * 0.7152f + b_f * 0.0722f);
    // Schaal naar 0-255
    uint8_t gray_8bit = static_cast<uint8_t>(gray_f * 255.0f);

    // Converteer 8-bit gray naar M5GFX kleur (RGB888 formaat)
    return M5.Display.color888(gray_8bit, gray_8bit, gray_8bit);
}

} // namespace m5papers3_display_m5gfx
} // namespace esphome
