#pragma once

#include "JuceHeader.h"

namespace ear {
namespace plugin {
namespace ui {

class EarSliderLabel : public Label {
public:
	EarSliderLabel(Slider& s);
	~EarSliderLabel();

	enum ColourIds { unitTextColourId = 0x1009280 };

	void textWasChanged() override;

	void setGrabFocusOnTextChange(bool grabFocus);
	bool getGrabFocusOnTextChange();

	void mouseDown(const MouseEvent& event) override;
	void mouseDrag(const MouseEvent& event) override;
	void mouseUp(const MouseEvent& event) override;
	MouseCursor getMouseCursor() override;

	void setUnit(const String& unit);
	String getUnit();

	void setUnitFont(const Font& newFont);
	Font getUnitFont();

	void paint(Graphics& g) override;

	void addListener(Slider::Listener* l) { listeners_.add(l); }
	void removeListener(Slider::Listener* l) { listeners_.remove(l); }

private:
	const float dragFactor_ = 0.002;
	String unit_ = "";
	Font unitFont_;
	bool grabFocus_ = true;
	double dragStartValue_;
	int dragStartPosY_;
	Slider& parentSlider_;

	ListenerList<Slider::Listener> listeners_;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarSliderLabel)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
