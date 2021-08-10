#pragma once

#include "JuceHeader.h"
#include "../../helper/singleton.hpp"
#include "../../binary_data.hpp"

namespace ear {
namespace plugin {
namespace ui {
namespace font {

class Roboto {
 public:
  Roboto() {
    robotoLight_ = Typeface::createSystemTypefaceFor(
        binary_data::RobotoLight_ttf, binary_data::RobotoLight_ttfSize);
    robotoRegular_ = Typeface::createSystemTypefaceFor(
        binary_data::RobotoRegular_ttf, binary_data::RobotoRegular_ttfSize);
    robotoMedium_ = Typeface::createSystemTypefaceFor(
        binary_data::RobotoMedium_ttf, binary_data::RobotoMedium_ttfSize);
  };

  /**
   * @note The font sizes in the following functions are acutal font sizes,
   * so from baseline to top and not the font height, which can be set using
   * juce. So `Roboto::getLight(14.f).getAscent()` returns 14.f!
   */

  Font getLight(float size = 12.f) {
    Font ret(robotoLight_);
    ret.setHeight(size * SIZE_HEIGHT_FACTOR);
    return ret;
  };
  Font getRegular(float size = 12.f) {
    Font ret(robotoRegular_);
    ret.setHeight(size * SIZE_HEIGHT_FACTOR);
    return ret;
  };
  Font getMedium(float size = 12.f) {
    Font ret(robotoMedium_);
    ret.setHeight(size * SIZE_HEIGHT_FACTOR);
    return ret;
  };

 private:
  Typeface::Ptr robotoLight_;
  Typeface::Ptr robotoRegular_;
  Typeface::Ptr robotoMedium_;

  const float SIZE_HEIGHT_FACTOR = 1.26315781358206891127338f / 1.08f;
};

typedef Singleton<Roboto> RobotoSingleton;

}  // namespace font
}  // namespace ui
}  // namespace plugin
}  // namespace ear
