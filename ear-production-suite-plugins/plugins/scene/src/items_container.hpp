#pragma once

#include <map>
#include <memory>

#include "JuceHeader.h"
#include "communication/common_types.hpp"
#include "metadata_listener.hpp"

namespace ear {
namespace plugin {
class ProgrammeObjects;
class ProgrammeObject;
class Metadata;
namespace proto {
  class InputItemMetadata;
  class ItemStore;
  class Programme;
}

namespace ui {

class ItemViewList;
class ItemView;
class EarButton;

class ItemsContainer : public Component,
        public MetadataListener {
 public:
  explicit ItemsContainer(Metadata& metadata);
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
  void createOrUpdateView(proto::InputItemMetadata const& item);
  void createOrUpdateViews(
      std::map<communication::ConnectionId,
      proto::InputItemMetadata> const& allItems);
  void removeView(communication::ConnectionId const& id);
  void themeItemsFor(ProgrammeObjects const& programme);
  void setMissingThemeFor(const communication::ConnectionId& id);
  void setPresentThemeFor(const communication::ConnectionId& id);

  // MetadataListener
  void dataReset (proto::ProgrammeStore const& programmes,
                  ItemMap const& items) override;
  void programmeSelected(ProgrammeObjects const& objects) override;
  void itemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& items) override;
  void itemRemovedFromProgramme(ProgrammeStatus status, communication::ConnectionId const& id) override;
  void inputRemoved(communication::ConnectionId const& id) override;
  void inputUpdated(const InputItem& item) override;
 private:
  Metadata& data_;
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
