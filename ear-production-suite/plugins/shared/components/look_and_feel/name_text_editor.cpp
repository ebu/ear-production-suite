#include "name_text_editor.hpp"
#include "colours.hpp"
#include "fonts.hpp"
#include "roboto.hpp"

using namespace ear::plugin::ui;

NameTextEditorLookAndFeel::NameTextEditorLookAndFeel() {
  setColour(TextEditor::backgroundColourId, EarColours::Transparent);
  setColour(TextEditor::textColourId, EarColours::Text);
  setColour(TextEditor::highlightColourId, EarColours::Primary);
  setColour(TextEditor::highlightedTextColourId, EarColours::Text);
  setColour(TextEditor::outlineColourId,
            EarColours::Text.withAlpha(Emphasis::medium));
  setColour(TextEditor::focusedOutlineColourId,
            EarColours::Text.withAlpha(Emphasis::medium));
  setColour(CaretComponent::caretColourId, EarColours::Text);
}

NameTextEditorLookAndFeel::~NameTextEditorLookAndFeel() {}

void NameTextEditorLookAndFeel::setLabelText(const String& labelText) {
  labelText_ = labelText;
}

void NameTextEditorLookAndFeel::fillTextEditorBackground(Graphics& g, int width,
                                                         int height,
                                                         TextEditor& e) {
  g.fillAll(findColour(TextEditor::backgroundColourId));
}

void NameTextEditorLookAndFeel::drawTextEditorOutline(Graphics& g, int width,
                                                      int height,
                                                      TextEditor& e) {
  const float xIndent = 0.5f * lineThickness;
  const float rectHeight = height - yTopIndent - 0.5f * lineThickness;
  const float rectWidth = width - 2 * xIndent;

  if (e.isEnabled() && e.hasKeyboardFocus(true) && !e.isReadOnly()) {
    g.setColour(findColour(TextEditor::highlightColourId));
  } else {
    g.setColour(findColour(TextEditor::outlineColourId));
  }

  g.setFont(EarFonts::Units);
  const float textWidth = EarFonts::Units.getStringWidthFloat(labelText_);
  g.drawSingleLineText(labelText_, textIndent,
                       yTopIndent + g.getCurrentFont().getHeight() * 0.1f);
  Path framePath;
  framePath.startNewSubPath(textIndent - labelMargin, yTopIndent);
  framePath.lineTo(xIndent + diameter / 2.f, yTopIndent);
  framePath.addArc(
      xIndent, yTopIndent, diameter, diameter, MathConstants<float>::twoPi,
      MathConstants<float>::pi + MathConstants<float>::halfPi, true);
  framePath.addArc(xIndent, yTopIndent + rectHeight - diameter, diameter,
                   diameter,
                   MathConstants<float>::pi + MathConstants<float>::halfPi,
                   MathConstants<float>::pi);
  framePath.addArc(xIndent + rectWidth - diameter,
                   yTopIndent + rectHeight - diameter, diameter, diameter,
                   MathConstants<float>::pi, MathConstants<float>::halfPi);
  framePath.addArc(xIndent + rectWidth - diameter, yTopIndent, diameter,
                   diameter, MathConstants<float>::halfPi, 0.f);
  framePath.lineTo(textIndent + textWidth + labelMargin, yTopIndent);
  g.strokePath(framePath, PathStrokeType(lineThickness));
}

// CaretComponent* createCaretComponent(Component* keyFocusOwner) {}
