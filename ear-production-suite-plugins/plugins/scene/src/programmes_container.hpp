#pragma once

#include <memory>
#include <mutex>
#include <optional>

#include "JuceHeader.h"
#include "components/ear_tabbed_component.hpp"
#include "communication/common_types.hpp"
#include "metadata_listener.hpp"
#include "store_metadata.hpp"
#include "object_view.hpp"
#include "programme_view.hpp"
#include "elements_container.hpp"

namespace ear::plugin {
class LevelMeterCalculator;

namespace ui {

class JuceSceneFrontendConnector;
class ProgrammeView;
class ElementViewList;
class Overlay;

class ProgrammesContainer : public juce::Component,
                            public MetadataListener,
                            public ObjectView::Listener,
                            public ProgrammeView::Listener,
                            public ElementsContainer::Listener,
                            public EarTabbedComponent::Listener {
 public:
  class Listener;
  ProgrammesContainer(std::shared_ptr<LevelMeterCalculator> meterCalculator,
                      Metadata& data);
  void resized() override;
  void addObjectView(
      int programmeIndex,
      const proto::InputItemMetadata& inputItem,
      const proto::Object& programmeElement);
  void clear();
  void updateElementOverview(ProgrammeObjects const& objects);
  void removeFromElementViews(communication::ConnectionId const& id);
  void addProgrammeView(const proto::Programme& programme);
  void moveProgrammeView(int oldIndex, int newIndex);
  void setProgrammeViewName(int programmeIndex,
                            const juce::String& newName);
  void setProgrammeViewLanguage(int programmeIndex, const std::optional<std::string>& language);
  int getProgrammeIndex(ProgrammeView* view) const;
  int getProgrammeIndex(ElementViewList* list) const;
  void addListener(Listener* listener);

// ObjectView::Listener
void objectDataChanged(ObjectView::Data data) override;

// ProgrammeView::Listener
void addItemClicked(ProgrammeView* view) override;
void nameChanged(ProgrammeView* view, const String& newName) override;
void languageChanged(ProgrammeView* view, int index) override;

// ElementsContainer::Listener
void elementMoved(ElementViewList* list, int oldIndex, int newIndex) override;
void removeElementClicked(ElementViewList* list, ElementView* view) override;

// EarTabbedComponent::Listener
void addTabClicked(EarTabbedComponent* tabbedComponent) override;
void tabSelectedId(EarTabbedComponent* tabbedComponent, const std::string& id) override;
void tabMovedId(EarTabbedComponent* tabbedComponent, const std::string& id,
              int newIndex) override;
void removeTabClickedId(EarTabbedComponent* tabbedComponent,
                        const std::string& id) override;
void tabBarDoubleClicked(EarTabbedComponent* tabbedComponent) override;

class Listener {
public:
    virtual ~Listener() = default;
    virtual void addItemClicked() = 0;
};

 private:
  std::shared_ptr<EarTabbedComponent> tabs_;
  std::vector<std::shared_ptr<ProgrammeView>> programmes_;
  std::shared_ptr<LevelMeterCalculator> meterCalculator_;
  Metadata& data_;
  std::vector<Listener*> listeners_;

  // MetadataListener
  void dataReset(const proto::ProgrammeStore &programmes, const ItemMap &items) override;
  void programmeAdded(ProgrammeStatus programmeIndex, proto::Programme const& programme) override;
  void programmeUpdated(ProgrammeStatus programmeIndex, proto::Programme const& programme) override;
  void itemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& items) override;
  void programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& item) override;
  void programmeMoved(Movement movement, proto::Programme const& programme) override;
  void programmeSelected(ProgrammeObjects const& objects) override;
  void programmeRemoved(int index) override;
  void itemRemovedFromProgramme(ProgrammeStatus status, communication::ConnectionId const& id) override;
  void inputRemoved(communication::ConnectionId const& id) override;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgrammesContainer)
};

}  // namespace ui
}  // namespace ear::plugin
