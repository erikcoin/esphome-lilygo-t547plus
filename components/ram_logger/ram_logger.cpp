#include "ram_logger.h"
#include "esphome/core/log.h"
#include "esp_log.h"

namespace esphome {
namespace ram_logger {

static constexpr size_t LOG_BUF_SIZE = 4096;

// 🔁 Change to RTC_DATA_ATTR if you want deep-sleep persistence
static char log_buffer[LOG_BUF_SIZE];
static size_t log_pos = 0;

static int ram_log_vprintf(const char *fmt, va_list args) {
  char temp[256];
  int len = vsnprintf(temp, sizeof(temp), fmt, args);

  if (len > 0) {
    size_t copy_len = std::min((size_t) len, LOG_BUF_SIZE - log_pos - 1);
    memcpy(&log_buffer[log_pos], temp, copy_len);
    log_pos += copy_len;
    log_buffer[log_pos] = '\0';
  }

  return len;
}

void RamLogger::setup() {
  esp_log_set_vprintf(ram_log_vprintf);
  ESP_LOGI("ram_logger", "RAM log buffer installed");
}

void RamLogger::dump() {
  ESP_LOGI("ram_logger", "Buffered logs:\n%s", log_buffer);
  log_pos = 0;
}

}  // namespace ram_logger
}  // namespace esphome
