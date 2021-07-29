#pragma once

#include "JuceHeader.h"

#include "ear_drop_indicator.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class ElementView;
class ElementsContainer;

class ElementViewList : public Component,
                        public DragAndDropTarget,
                        public DragAndDropContainer {
 public:
  ElementViewList(ElementsContainer* parentContainer);

  void paint(Graphics& g) override;

  void parentSizeChanged() override;

  void resized() override;

  void addElement(ElementView* element);

  void removeElement(ElementView* item);

  int getHeightOfAllItems() const;

  void moveElementTo(int oldIndex, int newIndex);

  bool isInterestedInDragSource(
    const SourceDetails& dragSourceDetails) override;

  void itemDragEnter(const SourceDetails& dragSourceDetails) override;

  int getDropIndexForPosition(int yPos);

  void itemDragMove(const SourceDetails& dragSourceDetails) override;

  void itemDropped(const SourceDetails& dragSourceDetails) override;

  void itemDragExit(const SourceDetails& dragSourceDetails) override;

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void elementMoved(ElementViewList* list, int oldIndex,
                              int newIndex) = 0;
    virtual void removeElementClicked(ElementViewList* list, int index) = 0;
  };

  void addListener(Listener* l);
  void removeListener(Listener* l);

 private:
  std::vector<ElementView*> elements_;
  std::unique_ptr<EarDropIndicator> dropIndicator_;
  std::unique_ptr<Label> helpLabel_;

  ElementsContainer* parentContainer;

  int dropIndex_ = 0;

  const int indicatorHeight_ = 6;
  const int margin_ = 1;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElementViewList)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
