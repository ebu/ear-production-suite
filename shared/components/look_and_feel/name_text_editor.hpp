#pragma once

#include "JuceHeader.h"

#include <map>

namespace ear {
namespace plugin {
namespace ui {

class NameTextEditorLookAndFeel : public LookAndFeel_V4 {
public:
	NameTextEditorLookAndFeel();
	~NameTextEditorLookAndFeel() override;

	void fillTextEditorBackground(Graphics& g, int width, int height,
		TextEditor& e) override;

	void drawTextEditorOutline(Graphics& g, int width, int height,
		TextEditor& e) override;

	void setLabelText(const String& l);

private:
	String labelText_ = "";

	const float textIndent = 13.f;
	const float diameter = 4.f;
	const float lineThickness = 1.f;
	const float yTopIndent = 10.0f;
	const float labelMargin = 4.f;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NameTextEditorLookAndFeel)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
