#pragma once

#include "JuceHeader.h"

#include "../helper/graphics.hpp"
#include "ear_button.hpp"
#include "ear_tab_button.hpp"
#include "ear_tabbed_component.hpp"

#include <memory>
#include <vector>

namespace ear {
namespace plugin {
namespace ui {

class EarTabButtonBarViewport : public Viewport {
 public:
  EarTabButtonBarViewport();
  void visibleAreaChanged(const juce::Rectangle<int>& newVisibleArea) override;
  std::function<void(const juce::Rectangle<int>& newVisibleArea)>
      onVisibleAreaChanged;

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarTabButtonBarViewport)
};

class EarTabButtonBarContent : public Component {
 public:
  EarTabButtonBarContent() = default;
  void mouseDoubleClick(const MouseEvent& event) override {
    if (onDoubleClick) {
      onDoubleClick();
    }
  }
  std::function<void()> onDoubleClick;

 private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarTabButtonBarContent)
};

struct EarTab;

class EarTabButtonBarIndicator : public Component {
 public:
  EarTabButtonBarIndicator();

  enum ColourIds { backgroundColourId = 0x68222cda };

  void setIcon(std::unique_ptr<Drawable> drawable);
  void paint(Graphics& g) override;

 private:
  std::unique_ptr<Drawable> drawable_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarTabButtonBarIndicator)
};

class EarTabButtonBar : public Component {
 public:
  EarTabButtonBar();

  void resized() override;

  void setTabs(std::vector<EarTab>* tabs);

  void addButtonAndMakeVisible(EarTabButton* button);
  void removeButton(EarTabButton* button);

  int updateTabBounds();
  void updateIndicatorVisibility();
  void scrollIfNecessaryTo(int index);

  std::function<void()> onDoubleClick;

 private:
  const int minButtonWidth_ = 128;
  const int maxButtonWidth_ = 258;
  const int buttonPadding_ = 2;

  std::unique_ptr<EarTabButtonBarIndicator> moreLeftIndicator_;
  std::unique_ptr<EarTabButtonBarIndicator> moreRightIndicator_;
  std::unique_ptr<EarTabButtonBarViewport> tabButtonBarViewport_;
  std::unique_ptr<EarTabButtonBarContent> tabButtonBarContent_;

  std::vector<EarTab>* tabs_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EarTabButtonBar)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
