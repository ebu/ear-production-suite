#pragma once

#include <map>
#include <memory>

#include "JuceHeader.h"

#include "communication/common_types.hpp"

namespace ear {
namespace plugin {
namespace proto {
  class InputItemMetadata;
  class ItemStore;
  class Programme;
}

namespace ui {

class ItemViewList;
class ItemView;
class EarButton;

class ItemsContainer : public Component {
 public:
  ItemsContainer();
  ~ItemsContainer();
  void paint(Graphics& g) override;
  void resized() override;


  class Listener {
   public:
    virtual ~Listener() = default;

    virtual void addItemsClicked(
        ItemsContainer* container,
        std::vector<communication::ConnectionId> ids) = 0;
  };

  void addListener(Listener* l) { listeners_.add(l); }
  void removeListener(Listener* l) { listeners_.remove(l); }
  void createOrUpdateViews(
      std::map<communication::ConnectionId,
      proto::InputItemMetadata> const& allItems);
  void updateView(communication::ConnectionId const& id,
                  proto::InputItemMetadata const& item);
  void removeView(communication::ConnectionId const& id);
  void themeItemsFor(proto::Programme const& programme);

 private:
  std::mutex mutex;
  std::unique_ptr<ItemViewList> objectsList;
  std::unique_ptr<ItemViewList> directSpeakersList;
  std::unique_ptr<ItemViewList> hoaList;
  std::vector<std::shared_ptr<ItemView>> objectsItems;
  std::vector<std::shared_ptr<ItemView>> directSpeakersItems;
  std::vector<std::shared_ptr<ItemView>> hoaItems;
  std::unique_ptr<Viewport> objectsViewport_;
  std::unique_ptr<Viewport> directSpeakersViewport_;
  std::unique_ptr<Viewport> hoaViewport_;
  std::unique_ptr<Label> objectsLabel_;
  std::unique_ptr<Label> directSpeakersLabel_;
  std::unique_ptr<Label> hoaLabel_;
  std::unique_ptr<EarButton> addButton_;

  ListenerList<Listener> listeners_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ItemsContainer)
};

}  // namespace ui
}  // namespace plugin
}  // namespace ear
