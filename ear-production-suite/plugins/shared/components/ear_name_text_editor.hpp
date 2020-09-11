#pragma once

#include "../helper/graphics.hpp"
#include "JuceHeader.h"
#include "look_and_feel/colours.hpp"
#include "look_and_feel/fonts.hpp"
#include "look_and_feel/name_text_editor.hpp"
#include "look_and_feel/slider.hpp"

namespace ear {
namespace plugin {
namespace ui {

class EarNameTextEditor : public TextEditor {
 public:
  EarNameTextEditor() : TextEditor() {
    setLookAndFeel(&earNameTextEditorLookAndFeel_);
    setFont(EarFonts::Items);
    setMultiLine(false);
    setJustification(Justification::left);
    setBorder(BorderSize<int>(0, textIndent, 0, textIndent));
  }

  void setLabelText(const String& l) {
    earNameTextEditorLookAndFeel_.setLabelText(l);
  }

  void resized() {
    // NOTE: for some weired reason this has to be called exactly here!!!
    setIndents(0.f, (getHeight() - yTopIndent - getFont().getHeight()) / 2 +
                        yTopIndent);
    TextEditor::resized();
  }

  void setText(const String& text, bool sendTextChangeMessage = true) {
    TextEditor::setText(text, sendTextChangeMessage);
    scrollToMakeSureCursorIsVisible();
  }

 private:
  // -- If changed make sure to sync changes with LookAndFeel
  const float textIndent = 13.f;
  const float yTopIndent = 10.0f;
  // --

  ear::plugin::ui::NameTextEditorLookAndFeel earNameTextEditorLookAndFeel_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarNameTextEditor)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
