#pragma once

#include "../../helper/singleton.hpp"
#include "JuceHeader.h"
#include "roboto.hpp"

namespace ear {
namespace plugin {
namespace ui {

struct EarFonts {
  EarFonts()
      : HeroHeading(font::RobotoSingleton::instance().getRegular(25.f)),
        Heading(font::RobotoSingleton::instance().getMedium(16.f)),
        Label(font::RobotoSingleton::instance().getRegular(14.f)),
        Items(font::RobotoSingleton::instance().getMedium(14.f)),
        ItemsLight(font::RobotoSingleton::instance().getLight(14.f)),
        ItemsSubtext(font::RobotoSingleton::instance().getLight(10.f)),
        Values(font::RobotoSingleton::instance().getMedium(12.f)),
        Description(font::RobotoSingleton::instance().getLight(12.f)),
        Units(font::RobotoSingleton::instance().getMedium(10.f)),
        Measures(font::RobotoSingleton::instance().getLight(10.f)),
        Version(font::RobotoSingleton::instance().getLight(10.f)),
        MinMaxLabel(font::RobotoSingleton::instance().getRegular(12.f)) {}

  const Font HeroHeading;
  const Font Heading;
  const Font Label;
  const Font Items;
  const Font ItemsLight;
  const Font ItemsSubtext;
  const Font Values;
  const Font Description;
  const Font Units;
  const Font Measures;
  const Font Version;
  const Font MinMaxLabel;
};

typedef Singleton<EarFonts> EarFontsSingleton;

}  // namespace ui
}  // namespace plugin
}  // namespace ear
