#pragma once

#include "JuceHeader.h"

namespace ear {
namespace plugin {
namespace ui {

namespace Emphasis {

static const float full = 1.f;
static const float high = 0.87f;
static const float medium = 0.6f;
static const float disabled = 0.38f;

}  // namespace Emphasis

namespace EarColours {

static const Colour Transparent = Colour(255, 255, 255).withAlpha(0.f);
static const Colour Background = Colour(18, 18, 18);
static const Colour Text = Colour(255, 255, 255);
static const Colour Primary = Colour(26, 93, 159);
static const Colour PrimaryVariant = Colour(0, 128, 255);
static const Colour PrimaryHighlight = Colour(207, 102, 121);
static const Colour Error = Colour(207, 102, 121);

static const Colour Area01dp = Colour(255, 255, 255).withAlpha(0.05f);
static const Colour Area02dp = Colour(255, 255, 255).withAlpha(0.07f);
static const Colour Area03dp = Colour(255, 255, 255).withAlpha(0.08f);
static const Colour Area04dp = Colour(255, 255, 255).withAlpha(0.09f);
static const Colour Area06dp = Colour(255, 255, 255).withAlpha(0.11f);
static const Colour Area08dp = Colour(255, 255, 255).withAlpha(0.12f);
static const Colour Area12dp = Colour(255, 255, 255).withAlpha(0.14f);
static const Colour Area16dp = Colour(255, 255, 255).withAlpha(0.15f);
static const Colour Area24dp = Colour(255, 255, 255).withAlpha(0.16f);

static const Colour Heading = Text.withAlpha(Emphasis::high);
static const Colour Label = Text.withAlpha(Emphasis::high);
static const Colour ComboBoxText = Text.withAlpha(Emphasis::high);
static const Colour WindowBorder = Colour(58, 58, 58);
static const Colour SliderTrack = Colour(165, 165, 165).withAlpha(0.16f);
static const Colour Shadow = Colour(255, 255, 255).withAlpha(0.015f);
static const Colour ThumbShadow = Colour(0, 0, 0).withAlpha(0.21f);
static const Colour Overlay = Colour(0, 0, 0).withAlpha(0.46f);
static const Colour HeaderText = Colour(0, 0, 0).withAlpha(0.87f);
static const Colour ComboBoxPopupBackground = Colour(49, 49, 49);
static const Colour ObjectItemBackground = Colour(30, 30, 30);
static const Colour Sphere = Colour(90, 90, 90);

static const Colour Item01 = Colour(255, 131, 111);
static const Colour Item02 = Colour(255, 255, 128);
static const Colour Item03 = Colour(0, 255, 128);
static const Colour Item04 = Colour(128, 255, 255);
static const Colour Item05 = Colour(255, 128, 255);
static const Colour Item06 = Colour(128, 128, 192);
static const Colour Item07 = Colour(255, 128, 64);
static const Colour Item08 = Colour(0, 128, 128);
static const Colour Item09 = Colour(128, 0, 64);
static const Colour Item10 = Colour(128, 0, 255);
static const Colour Item11 = Colour(0, 64, 0);
static const Colour Item12 = Colour(49, 142, 61);

static const std::vector<Colour> Items{Item01, Item02, Item03, Item04,
                                       Item05, Item06, Item07, Item08,
                                       Item09, Item10, Item11, Item12};

}  // namespace EarColours

}  // namespace ui
}  // namespace plugin
}  // namespace ear
