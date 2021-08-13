#pragma once

#include "JuceHeader.h"
#include "colours.hpp"

namespace ear {
namespace plugin {
namespace ui {

namespace Shadows {

static const DropShadow elevation01dp(EarColours::Shadow, 1,
                                      juce::Point<int>{0, 1});
static const DropShadow elevation02dp(EarColours::Shadow, 2,
                                      juce::Point<int>{0, 2});
static const DropShadow elevation03dp(EarColours::Shadow, 4,
                                      juce::Point<int>{0, 3});
static const DropShadow elevation04dp(EarColours::Shadow, 5,
                                      juce::Point<int>{0, 4});
static const DropShadow elevation06dp(EarColours::Shadow, 10,
                                      juce::Point<int>{0, 6});
static const DropShadow elevation08dp(EarColours::Shadow, 10,
                                      juce::Point<int>{0, 8});
static const DropShadow elevation12dp(EarColours::Shadow, 17,
                                      juce::Point<int>{0, 12});
static const DropShadow elevation16dp(EarColours::Shadow, 24,
                                      juce::Point<int>{0, 16});
static const DropShadow elevation24dp(EarColours::Shadow, 38,
                                      juce::Point<int>{0, 24});

static const DropShadow thumb(EarColours::ThumbShadow, 3,
                              juce::Point<int>{0, 1});

}  // namespace Shadows
}  // namespace ui
}  // namespace plugin
}  // namespace ear
