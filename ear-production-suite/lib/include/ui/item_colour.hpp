#pragma once

#include <stdint.h>

namespace ear {
namespace plugin {
namespace ui {

class ItemColour {
 public:
  ItemColour() : argbValue_(0){};
  explicit ItemColour(uint32_t argb) : argbValue_(argb){};
  explicit ItemColour(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
      : argbValue_(((alpha << 24) | (red << 16) | (green << 8) | blue)) {}

  bool operator==(const ItemColour& rhs) const {
    return rhs.argbValue_ == argbValue_;
  }
  bool operator!=(const ItemColour& rhs) const {
    return rhs.argbValue_ != argbValue_;
  }

  uint32_t argbValue() const { return argbValue_; }

  uint8_t red() const { return (argbValue_ >> 16) & 0xFF; }
  uint8_t green() const { return (argbValue_ >> 8) & 0xFF; }
  uint8_t blue() const { return argbValue_ & 0xFF; }
  uint8_t alpha() const { return (argbValue_ >> 24) & 0xFF; }

 private:
  int32_t argbValue_;
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
