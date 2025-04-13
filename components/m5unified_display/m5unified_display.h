#include "esphome.h"
#include <M5Unified.h>

class M5UnifiedDisplay : public PollingComponent, public DisplayBuffer {
 public:
  M5UnifiedDisplay() : PollingComponent(1000) {}

  void setup() override {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.fillScreen(BLACK);
    M5.Display.setTextColor(WHITE);
    M5.Display.setTextSize(2);
  }

  void update() override {
    this->do_display();
  }

  void draw_absolute_pixel_internal(int x, int y, Color color) override {
    M5.Display.drawPixel(x, y, color.raw_888());
  }

  int get_width_internal() override { return M5.Display.width(); }
  int get_height_internal() override { return M5.Display.height(); }

  void display() override {
    M5.Display.startWrite();
    DisplayBuffer::display();
    M5.Display.endWrite();
  }

  void fill(Color color) override {
    M5.Display.fillScreen(color.raw_888());
  }

  void draw_pixel_at(int x, int y, Color color) {
    M5.Display.drawPixel(x, y, color.raw_888());
  }
};
