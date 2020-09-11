#pragma once

#include "JuceHeader.h"

#include "../../shared/components/ear_colour_indicator.hpp"
#include "../../shared/components/level_meter.hpp"
#include "../../shared/components/look_and_feel/colours.hpp"
#include "../../shared/components/look_and_feel/fonts.hpp"
#include "ear_drop_indicator.hpp"

#include <memory>

namespace ear {
namespace plugin {
namespace ui {

class ItemViewList : public Component {
 public:
  ItemViewList() {}

  void paint(Graphics& g) override { g.fillAll(EarColours::Background); }

  void parentSizeChanged() override {
    setSize(getParentWidth(),
            std::max(getParentHeight(), getHeightOfAllItems()));
  }

  void resized() override {
    setSize(getParentWidth(),
            std::max(getParentHeight(), getHeightOfAllItems()));
    auto area = getLocalBounds();
    for (int i = 0; i < items_.size(); ++i) {
      items_.at(i)->setBounds(
          area.removeFromTop(items_.at(i)->getDesiredHeight()));
      area.removeFromTop(margin_);
    }
  }

  void addItem(ItemView* item) {
    items_.push_back(item);
    addAndMakeVisible(item);
    resized();
  }

  int getHeightOfAllItems() const {
    int ret = 0;
    for (const auto item : items_) {
      ret += item->getDesiredHeight() + margin_;
    }
    return ret;
  }

  void moveItemTo(ItemView* item, int index) {
    auto it = std::find(items_.begin(), items_.end(), item);
    size_t sourceIndex = std::distance(items_.begin(), it);
    index = index > sourceIndex ? index - 1 : index;
    if (it != items_.end()) {
      items_.erase(it);
    }
    if (index < items_.size()) {
      items_.insert(items_.begin() + index, item);
    } else {
      items_.push_back(item);
    }
  }

  void removeItem(ItemView* item) {
    auto it = std::find(items_.begin(), items_.end(), item);
    if (it != items_.end()) {
      items_.erase(it);
      resized();
    }
  }

  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void itemAdded(ItemViewList* list, ItemView* view) = 0;
    virtual void itemMoved(ItemViewList* list, int index);
    virtual void removeItemClicked(ItemViewList* list, int index) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }

 private:
  std::vector<ItemView*> items_;

  const int margin_ = 1;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ItemViewList)
};  // namespace ui

}  // namespace ui
}  // namespace plugin
}  // namespace ear
