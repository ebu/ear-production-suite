#pragma once

#include "JuceHeader.h"

#include "ear_drop_indicator.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class ElementsContainer;

class ElementViewList : public Component,
                        public DragAndDropTarget,
                        public DragAndDropContainer {
 public:
  ElementViewList(ElementsContainer* parentContainer);

  void paint(Graphics& g) override;

  void parentSizeChanged() override;

  void resized() override;

  int getHeightOfAllItems() const;

  bool isInterestedInDragSource(
    const SourceDetails& dragSourceDetails) override;

  void itemDragEnter(const SourceDetails& dragSourceDetails) override;

  int getDropIndexForPosition(int yPos);

  void itemDragMove(const SourceDetails& dragSourceDetails) override;

  void itemDropped(const SourceDetails& dragSourceDetails) override;

  void itemDragExit(const SourceDetails& dragSourceDetails) override;

 private:
  std::unique_ptr<EarDropIndicator> dropIndicator_;
  std::unique_ptr<Label> helpLabel_;

  ElementsContainer* parentContainer;

  int dropIndex_ = 0;

  const int indicatorHeight_ = 6;
  const int margin_ = 1;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElementViewList)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
