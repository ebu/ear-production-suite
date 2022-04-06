#pragma once

#include <memory>
#include <mutex>
#include <optional>

#include "JuceHeader.h"
#include "components/ear_tabbed_component.hpp"
#include "communication/common_types.hpp"
#include "metadata_listener.hpp"

namespace ear::plugin {
class LevelMeterCalculator;

namespace ui {

class JuceSceneFrontendConnector;
class ProgrammeView;
class ElementViewList;
class ObjectView;

class ProgrammesContainer : public juce::Component, public MetadataListener {
 public:
  ProgrammesContainer();
  void resized() override;
  std::shared_ptr<ObjectView> addObjectView(
      int programmeIndex,
      const proto::InputItemMetadata& inputItem,
      const proto::Object& programmeElement,
      std::shared_ptr<LevelMeterCalculator> const& meterCalculator);
  void clear();
  void removeListeners(JuceSceneFrontendConnector* connector);
  void addTabListener(EarTabbedComponent::Listener* listener);
  void updateElementOverview(ProgrammeObjects const& objects);
  int programmeCount() const;
  void updateViews(proto::InputItemMetadata const& item,
                   std::shared_ptr<LevelMeterCalculator> const& meterCalculator);
  void removeFromElementViews(communication::ConnectionId const& id);
  void addProgrammeView(const proto::Programme& programme,
                        JuceSceneFrontendConnector& connector);
  void selectTab(int index);
  void moveProgrammeView(int oldIndex, int newIndex);
  void removeProgrammeView(int index);
  void setProgrammeViewName(int programmeIndex,
                            const juce::String& newName);
  void setProgrammeViewLanguage(int programmeIndex, const std::optional<std::string>& language);
  int getProgrammeIndex(ProgrammeView* view) const;
  int getProgrammeIndex(ElementViewList* list) const;

 private:
  std::shared_ptr<EarTabbedComponent> tabs_;
  std::vector<std::shared_ptr<ProgrammeView>> programmes_;
  void itemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& items) override;
  void itemRemovedFromProgramme(ProgrammeStatus status, communication::ConnectionId const& id) override;
  void programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& item) override;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgrammesContainer)
};

}  // namespace ui
}  // namespace ear::plugin
