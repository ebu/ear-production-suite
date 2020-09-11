#pragma once

#include "JuceHeader.h"

#include "helper/move.hpp"
#include "../../shared/components/ear_colour_indicator.hpp"
#include "../../shared/components/level_meter.hpp"
#include "../../shared/components/look_and_feel/colours.hpp"
#include "../../shared/components/look_and_feel/fonts.hpp"
#include "ear_drop_indicator.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class ElementViewList : public Component,
                        public DragAndDropTarget,
                        public DragAndDropContainer {
 public:
  ElementViewList()
      : dropIndicator_(std::make_unique<EarDropIndicator>()),
        helpLabel_(std::make_unique<Label>()) {
    addChildComponent(dropIndicator_.get());

    helpLabel_->setText("Add Items by clicking the Buttons above",
                        dontSendNotification);
    helpLabel_->setFont(EarFonts::Items);
    helpLabel_->setColour(Label::textColourId,
                          EarColours::Text.withAlpha(Emphasis::medium));
    addAndMakeVisible(helpLabel_.get());
  }

  void paint(Graphics& g) override { g.fillAll(EarColours::Background); }

  void parentSizeChanged() override {
    setSize(getParentWidth(),
            std::max(getParentHeight(), getHeightOfAllItems()));
  }

  void resized() override {
    setSize(getParentWidth(),
            std::max(getParentHeight(), getHeightOfAllItems()));
    auto labelBounds = getLocalBounds().removeFromTop(60);
    helpLabel_->setBounds(labelBounds.reduced(30, 20));
    auto area = getLocalBounds();
    for (int i = 0; i < elements_.size(); ++i) {
      if (dropIndicator_->isVisible() && i == dropIndex_) {
        dropIndicator_->setBounds(
            area.removeFromTop(indicatorHeight_).reduced(0, 2));
        area.removeFromTop(margin_);
      }
      elements_.at(i)->setBounds(
          area.removeFromTop(elements_.at(i)->getDesiredHeight()));
      area.removeFromTop(margin_);
    }
    if (dropIndicator_->isVisible() && dropIndex_ == elements_.size()) {
      dropIndicator_->setBounds(
          area.removeFromTop(indicatorHeight_).reduced(0, 2));
      area.removeFromTop(margin_);
    }
  }

  void addElement(ElementView* element) {
    elements_.push_back(element);
    element->getRemoveButton()->onClick = [this, element]() {
      auto it =
          std::find(this->elements_.begin(), this->elements_.end(), element);
      auto index = std::distance(this->elements_.begin(), it);
      Component::BailOutChecker checker(this);
      listeners_.callChecked(checker, [this, index](Listener& l) {
        l.removeElementClicked(this, index);
      });
      if (checker.shouldBailOut()) {
        return;
      }
    };
    addAndMakeVisible(element);
    helpLabel_->setVisible(elements_.size() == 0);
    resized();
  }

  void removeElement(ElementView* item) {
    auto it = std::find(elements_.begin(), elements_.end(), item);
    if (it != elements_.end()) {
      elements_.erase(it);
      helpLabel_->setVisible(elements_.size() == 0);
      resized();
    }
  }

  int getHeightOfAllItems() const {
    int ret = 0;
    for (const auto element : elements_) {
      ret += element->getDesiredHeight() + margin_;
    }
    if (dropIndicator_->isVisible()) {
      ret += indicatorHeight_ + margin_;
    }
    return ret;
  }

  void moveElementTo(int oldIndex, int newIndex) {
    if (oldIndex < elements_.size() && newIndex < elements_.size() &&
        oldIndex != newIndex) {
      move(elements_.begin(), oldIndex, newIndex);
    }
    Component::BailOutChecker checker(this);
    listeners_.callChecked(checker, [this, oldIndex, newIndex](Listener& l) {
      l.elementMoved(this, oldIndex, newIndex);
    });
    if (checker.shouldBailOut()) {
      return;
    }
  }

  bool isInterestedInDragSource(
      const SourceDetails& dragSourceDetails) override {
    return true;
  };

  void itemDragEnter(const SourceDetails& dragSourceDetails) override {
    dropIndicator_->setVisible(true);
  }

  int getDropIndexForPosition(int yPos) {
    int y = 0;
    int previousHeight = 0;
    int currentHeight = 0;
    for (int i = 0; i < elements_.size(); ++i) {
      currentHeight = elements_.at(i)->getHeight();
      if (y - 0.5f * previousHeight < yPos &&
          yPos < y + 0.51f * currentHeight) {
        return i;
      }
      y += currentHeight;
      previousHeight = currentHeight;
    }
    return elements_.size();
  }

  void itemDragMove(const SourceDetails& dragSourceDetails) override {
    dropIndex_ =
        getDropIndexForPosition(dragSourceDetails.localPosition.getY());
    resized();
  }

  void itemDropped(const SourceDetails& dragSourceDetails) override {
    if (auto component = dragSourceDetails.sourceComponent.get()) {
      if (auto element = dynamic_cast<ElementView*>(component)) {
        auto it = std::find(elements_.begin(), elements_.end(), element);
        size_t oldIndex = std::distance(elements_.begin(), it);
        int newIndex = dropIndex_ > oldIndex ? dropIndex_ - 1 : dropIndex_;
        moveElementTo(oldIndex, newIndex);
      }
    }
    dropIndicator_->setVisible(false);
    resized();
  }

  void itemDragExit(const SourceDetails& dragSourceDetails) override {
    dropIndicator_->setVisible(false);
    resized();
  }

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void elementMoved(ElementViewList* list, int oldIndex,
                              int newIndex) = 0;
    virtual void removeElementClicked(ElementViewList* list, int index) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

 private:
  std::vector<ElementView*> elements_;
  std::unique_ptr<EarDropIndicator> dropIndicator_;
  std::unique_ptr<Label> helpLabel_;

  int dropIndex_ = 0;

  const int indicatorHeight_ = 6;
  const int margin_ = 1;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ElementViewList)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
