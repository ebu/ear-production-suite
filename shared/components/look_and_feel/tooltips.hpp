#pragma once

#include "JuceHeader.h"
#include "colours.hpp"

namespace ear {
namespace plugin {
namespace ui {

class TooltipLookAndFeel : public LookAndFeel_V4
{
public:
    TooltipLookAndFeel()
    {
        setColour(TooltipWindow::backgroundColourId, EarColours::Primary.darker());
        setColour(TooltipWindow::textColourId, juce::Colours::white);
    }

    juce::Rectangle<int> getTooltipBounds (const String& tipText, Point<int> screenPos, juce::Rectangle<int> parentArea) override
    {
        const TextLayout tl (layoutTooltipText (tipText, Colours::black));

        auto w = (int) (tl.getWidth() + 14.0f);
        auto h = (int) (tl.getHeight() + 6.0f);

        auto ret = juce::Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                                   screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6)  : screenPos.y + 6,
                                   w, h)
            .constrainedWithin (parentArea);
        return ret;

    }

    TextLayout layoutTooltipText (const String& text, Colour colour) noexcept
    {
        const float tooltipFontSize = 13.0f;
        const int maxToolTipWidth = 350;

        AttributedString s;
        s.setJustification (Justification::centred);
        s.append (text, Font (tooltipFontSize, Font::plain), colour);

        TextLayout tl;
        tl.createLayoutWithBalancedLineLengths (s, (float) maxToolTipWidth);
        return tl;
    }

    void drawTooltip (Graphics& g, const String& text, int width, int height) override
    {
        juce::Rectangle<int> bounds (width, height);
        auto cornerSize = 5.0f;

        g.setColour (findColour (TooltipWindow::backgroundColourId));
        g.fillRoundedRectangle (bounds.toFloat(), cornerSize);

        g.setColour (findColour (TooltipWindow::outlineColourId));
        g.drawRoundedRectangle (bounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);

        layoutTooltipText (text, findColour (TooltipWindow::textColourId))
            .draw (g, { static_cast<float> (width), static_cast<float> (height) });
    }

};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
