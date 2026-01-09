#pragma once

#include "esphome/core/component.h"

namespace esphome {
namespace ram_logger {

class RamLogger : public Component {
 public:
  void setup() override;
  void dump();
};

}  // namespace ram_logger
}  // namespace esphome
