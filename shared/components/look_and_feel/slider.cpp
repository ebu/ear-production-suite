#include "slider.hpp"
#include "../../helper/graphics.hpp"
#include "../ear_slider_label.hpp"
#include "../ear_slider.hpp"
#include "colours.hpp"
#include "fonts.hpp"
#include "roboto.hpp"

using namespace ear::plugin::ui;

SliderLookAndFeel::SliderLookAndFeel() {
	setColour(CaretComponent::caretColourId, EarColours::Text);
	setColour(Slider::backgroundColourId, EarColours::Transparent);
	setColour(Slider::thumbColourId, EarColours::Text);
	setColour(Slider::trackColourId, Colour(0x29a5a5a5));

	setColour(Slider::textBoxTextColourId, EarColours::Text);
	setColour(Slider::textBoxBackgroundColourId, EarColours::Area01dp);
	setColour(Slider::textBoxHighlightColourId, EarColours::Primary);
	setColour(Slider::textBoxOutlineColourId, EarColours::Primary);
}

SliderLookAndFeel::~SliderLookAndFeel() {}

void SliderLookAndFeel::setTicks(std::vector<Tick> ticks) { ticks_ = ticks; }
void SliderLookAndFeel::setUnit(const String& unit) { unit_ = unit; }
void SliderLookAndFeel::setValueOrigin(float valueOrigin) {
	valueOrigin_ = valueOrigin;
}
void SliderLookAndFeel::setGrabFocusOnTextChange(bool grabFocus) {
	grabFocus_ = grabFocus;
}

void SliderLookAndFeel::drawLinearSlider(Graphics& g, int x, int y, int width,
	int height, float sliderPos,
	float minSliderPos, float maxSliderPos,
	const Slider::SliderStyle style,
	Slider& slider) {
	auto textBoxBounds = getSliderLayout(slider).textBoxBounds;
	// g.setColour(findColour(Slider::textBoxBackgroundColourId));
	// g.fillRect(textBoxBounds);
	// if (slider.hasKeyboardFocus(true)) {
	//   g.setColour(findColour(Slider::textBoxHighlightColourId));
	//   g.drawRect(textBoxBounds, 1.f);
	// }
	drawLinearSliderBackground(g, x, y, width, height, sliderPos, minSliderPos,
		maxSliderPos, style, slider);
	drawLinearSliderThumb(g, x, y, width, height, sliderPos, minSliderPos,
		maxSliderPos, style, slider);
}

void SliderLookAndFeel::drawLinearSliderBackground(
	Graphics& g, int x, int y, int width, int height, float sliderPos,
	float minSliderPos, float maxSliderPos, const Slider::SliderStyle style,
	Slider& slider) {
	g.fillAll(findColour(Slider::backgroundColourId));

	switch (slider.getSliderStyle()) {
	case Slider::LinearHorizontal: {
		const float trackPosY = height - sliderRadius_;
		g.setColour(findColour(Slider::trackColourId));
		fillRoundedRectangle(g, x, trackPosY - trackWidth_ / 2.f, width,
			trackWidth_, 1.f);

		float valueOriginPos = slider.getPositionOfValue(valueOrigin_);
		g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
		fillRoundedRectangle(g, valueOriginPos, trackPosY - trackWidth_ / 2.f,
			sliderPos - valueOriginPos, trackWidth_, 1.f);

		g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
		g.setFont(EarFonts::Measures);
		for (const auto tick : ticks_) {
			float tickPos = slider.getPositionOfValue(tick.value);
			g.drawLine(tickPos, trackPosY - trackWidth_ / 2.f - tickMargin_,
				tickPos,
				trackPosY - trackWidth_ / 2.f - tickMargin_ - tickLength_,
				tickWidth_);
			auto justification = Justification::centred;
			juce::Rectangle<float> labelRect(tickPos - tickLabelWidth_ / 2.f,
				trackPosY - tickMargin_ - tickLength_ -
				tickMargin_ - tickLabelHeight_,
				tickLabelWidth_, tickLabelHeight_);
			if (tick.justification == Justification::left) {
				labelRect.translate(tickLabelWidth_ / 2.f, 0.f);
			}
			else if (tick.justification == Justification::right) {
				labelRect.translate(-tickLabelWidth_ / 2.f, 0.f);
			}
			g.drawText(tick.text, labelRect, tick.justification);
		}
	} break;
	case Slider::LinearVertical: {
		const float trackPosX = width - sliderRadius_;
		g.setColour(findColour(Slider::trackColourId));
		fillRoundedRectangle(g, trackPosX - trackWidth_ / 2.f, y, trackWidth_,
			height, 1.f);
		g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
		float valueOriginPos = slider.getPositionOfValue(valueOrigin_);
		fillRoundedRectangle(g, trackPosX - trackWidth_ / 2.f, valueOriginPos,
			trackWidth_, sliderPos - valueOriginPos, 1.f);

		g.setColour(EarColours::Text.withAlpha(Emphasis::medium));
		g.setFont(EarFonts::Measures);
		for (const auto tick : ticks_) {
			float tickPos = slider.getPositionOfValue(tick.value);
			g.drawLine(trackPosX - trackWidth_ / 2.f - tickMargin_, tickPos,
				trackPosX - trackWidth_ / 2.f - tickMargin_ - tickLength_,
				tickPos, tickWidth_);
			juce::Rectangle<float> labelRect(
				trackPosX - trackWidth_ / 2.f - tickMargin_ - tickLength_ -
				tickLabelWidth_ - 0.3f * EarFonts::Measures.getHeight(),
				tickPos - tickLabelHeight_ / 2.f, tickLabelWidth_,
				tickLabelHeight_);
			if (tick.justification == Justification::topRight) {
				labelRect.translate(0.f, tickLabelHeight_ / 2.f);
			}
			else if (tick.justification == Justification::bottomRight) {
				labelRect.translate(0.f, -tickLabelHeight_ / 2.f);
			}
			g.drawText(tick.text, labelRect, tick.justification);
		}
	} break;
	case Slider::IncDecButtons: {
		// intentionally empty
	} break;
	default:
		break;
	}
}

void SliderLookAndFeel::drawLinearSliderThumb(
	Graphics& g, int x, int y, int width, int height, float sliderPos,
	float minSliderPos, float maxSliderPos, const Slider::SliderStyle style,
	Slider& slider) {
	g.setColour(findColour(Slider::thumbColourId));
	switch (slider.getSliderStyle()) {
	case Slider::LinearHorizontal:
		g.fillEllipse(sliderPos - sliderRadius_, height - 2.f * sliderRadius_,
			2.0f * sliderRadius_, 2.0f * sliderRadius_);
		break;
	case Slider::LinearVertical:
		g.fillEllipse(width - 2.f * sliderRadius_, sliderPos - sliderRadius_,
			2.0f * sliderRadius_, 2.0f * sliderRadius_);
		break;
	case Slider::IncDecButtons: {
		// intentionally empty
	} break;
	default:
		break;
	}
}

Slider::SliderLayout SliderLookAndFeel::getSliderLayout(Slider& slider) {
	Slider::SliderLayout layout;
	auto bounds = slider.getLocalBounds();
	if (slider.getSliderStyle() == Slider::IncDecButtons) {
		layout.textBoxBounds = bounds;
	}
	else {
		switch (slider.getTextBoxPosition()) {
		case Slider::NoTextBox:
			layout.sliderBounds = bounds;
			break;
		case Slider::TextBoxLeft:
			layout.textBoxBounds =
				bounds.removeFromLeft(44).withSizeKeepingCentre(44, 40);
			layout.sliderBounds = bounds.withTrimmedLeft(10).withTrimmedRight(8);
			break;
		case Slider::TextBoxRight:
			layout.textBoxBounds =
				bounds.removeFromRight(44).withSizeKeepingCentre(44, 40);
			layout.sliderBounds = bounds.withTrimmedRight(10).withTrimmedLeft(8);
			break;
		case Slider::TextBoxAbove:
			layout.textBoxBounds =
				bounds.removeFromTop(40).withSizeKeepingCentre(44, 40);
			layout.sliderBounds = bounds.withTrimmedTop(10).withTrimmedBottom(8);
			break;
		case Slider::TextBoxBelow:
			layout.textBoxBounds =
				bounds.removeFromBottom(40).withSizeKeepingCentre(44, 40);
			layout.sliderBounds = bounds.withTrimmedBottom(10).withTrimmedTop(8);
			break;
		}
	}
	return layout;
}

Label* SliderLookAndFeel::createSliderTextBox(Slider& slider) {
	auto l = new EarSliderLabel(slider);
	if (unit_.isNotEmpty()) {
		l->setBorderSize(BorderSize<int>(0, 0, 10, 0));
	}
	else {
		l->setBorderSize(BorderSize<int>(0));
	}
	l->setFont(EarFonts::Values);
	l->setUnitFont(EarFonts::Units);
	l->setUnit(unit_);
	l->setJustificationType(Justification::centred);
	l->setKeyboardType(TextInputTarget::decimalKeyboard);
	l->setColour(Label::textColourId,
		slider.findColour(Slider::textBoxTextColourId));
	l->setColour(Label::backgroundColourId,
		slider.findColour(Slider::textBoxBackgroundColourId));
	l->setColour(Label::outlineColourId, EarColours::Primary);
	l->setColour(EarSliderLabel::unitTextColourId,
		EarColours::Text.withAlpha(Emphasis::medium));
	l->onEditorShow = [this, l] {
		auto* e = l->getCurrentTextEditor();
		e->setLookAndFeel(this);
		e->setJustification(Justification::centredTop);
		e->setFont(EarFonts::Values);
		e->setColour(TextEditor::highlightColourId,
			EarColours::Text.withAlpha(Emphasis::disabled));
		e->setColour(TextEditor::focusedOutlineColourId, EarColours::Primary);
		int topIndent;
		int leftIndent;
		if (this->unit_.isNotEmpty()) {
			topIndent = (e->getHeight() - 10.f - e->getFont().getHeight()) / 2;
			leftIndent = 4.f;
		}
		else {
			topIndent = (e->getHeight() - 1.f - e->getFont().getHeight()) / 2;
			leftIndent = 4.f;
		}
		e->setIndents(leftIndent, topIndent);
		e->resized();
	};
	auto earSlider = dynamic_cast<EarSlider*>(&slider);
	if (earSlider) {
		earSlider->setLabel(l);
	}
	return l;
}

// void SliderLookAndFeel::drawLabel(Graphics& g, Label& label) {
//   g.fillAll(label.findColour(Label::backgroundColourId));

//   if (!label.isBeingEdited()) {
//     const Font font(getLabelFont(label));
//     g.setColour(label.findColour(Label::textColourId));
//     g.setFont(font);

//     auto textArea =
//         getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

//     g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
//                      jmax(1, (int)(textArea.getHeight() / font.getHeight())),
//                      label.getMinimumHorizontalScale());
//   }
//   if (label.hasKeyboardFocus(true)) {
//     g.setColour(label.findColour(Label::outlineColourId));
//   } else {
//     g.setColour(label.findColour(Label::backgroundColourId));
//   }
//   g.drawRect(label.getLocalBounds(), 1);
// }

void SliderLookAndFeel::drawTextEditorOutline(Graphics& g, int width,
	int height,
	TextEditor& textEditor) {
	if (textEditor.isEnabled()) {
		if (textEditor.hasKeyboardFocus(true) && !textEditor.isReadOnly()) {
			const int border = 1;

			g.setColour(textEditor.findColour(TextEditor::focusedOutlineColourId));
			g.drawRect(0, 0, width, height, border);

			g.setOpacity(1.0f);
			auto shadowColour = textEditor.findColour(TextEditor::shadowColourId)
				.withMultipliedAlpha(0.75f);
			drawBevel(g, 0, 0, width, height + 2, border + 2, shadowColour,
				shadowColour);
		}
		else {
			g.setColour(textEditor.findColour(TextEditor::outlineColourId));
			g.drawRect(0, 0, width, height);

			g.setOpacity(1.0f);
			auto shadowColour = textEditor.findColour(TextEditor::shadowColourId);
			drawBevel(g, 0, 0, width, height + 2, 3, shadowColour, shadowColour);
		}
	}
}
