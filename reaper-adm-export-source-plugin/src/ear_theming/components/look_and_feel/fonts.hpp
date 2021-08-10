#pragma once

#include "JuceHeader.h"
#include "roboto.hpp"

namespace ear {
namespace plugin {
namespace ui {

namespace EarFonts {

static const Font HeroHeading =
    font::RobotoSingleton::instance().getRegular(25.f);
static const Font Heading = font::RobotoSingleton::instance().getMedium(16.f);
static const Font Label = font::RobotoSingleton::instance().getRegular(14.f);
static const Font Items = font::RobotoSingleton::instance().getMedium(14.f);
static const Font Values = font::RobotoSingleton::instance().getMedium(12.f);
static const Font Description =
    font::RobotoSingleton::instance().getLight(12.f);
static const Font Units = font::RobotoSingleton::instance().getMedium(10.f);
static const Font Measures = font::RobotoSingleton::instance().getLight(10.f);

}  // namespace EarFonts

}  // namespace ui
}  // namespace plugin
}  // namespace ear
