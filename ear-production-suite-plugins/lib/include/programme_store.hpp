//
// Created by Richard Bailey on 23/02/2022.
//

#ifndef EAR_PRODUCTION_SUITE_PROGRAMME_STORE_HPP
#define EAR_PRODUCTION_SUITE_PROGRAMME_STORE_HPP
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "metadata.hpp"

namespace ear::plugin {
class ItemStore;
class Metadata;
namespace communication {
class ConnectionId;
}

class ProgrammeStore {
 public:
  explicit ProgrammeStore(Metadata& metadata) : metadata_{metadata} {}

  [[ nodiscard ]] proto::ProgrammeStore const& get() const;
  [[ nodiscard ]] std::optional<proto::Programme> selectedProgramme() const;
  [[ nodiscard ]] std::optional<proto::Programme> programmeAtIndex(int index) const;
  [[ nodiscard ]] int programmeCount() const;
  [[ nodiscard ]] std::string programmeName(int index) const;;
  [[ nodiscard ]] bool autoModeEnabled() const;

  void set(proto::ProgrammeStore const& store);
  void setAutoMode(bool enable);
  void selectProgramme(int index);
  void addProgramme();
  void moveProgramme(int oldIndex, int newIndex);
  void removeProgramme(int index);
  void setProgrammeName(int index, std::string const& name);
  void setProgrammeLanguage(int programmeIndex, std::string const& language);
  void clearProgrammeLanguage(int programmeIndex);
  void addItemsToSelectedProgramme(std::vector<communication::ConnectionId> const& id);
  void updateElement(communication::ConnectionId const& id, proto::Object const& element);
  void removeElementFromProgramme(int programmeIndex, communication::ConnectionId const& id);
  void removeElementFromAllProgrammes(communication::ConnectionId const& id);
  void moveElement(int programmeIndex, int oldIndex, int newIndex);
  void autoUpdateFrom(ItemStore const& itemStore);

 private:
  void removeElementFromProgramme(int programmeIndex, int elementIndex);
  std::optional<proto::Programme> programmeAtIndexImpl(int index) const;
  proto::Programme* addProgrammeImpl(std::string const& name, std::string const& language);
  proto::Object* addObject(
      proto::Programme* programme,
      const communication::ConnectionId id);

  Metadata& metadata_;
  proto::ProgrammeStore store_;
};

[[ nodiscard ]]
bool isItemInProgramme(communication::ConnectionId const& id,
                       proto::Programme const& programme);
}

#endif  // EAR_PRODUCTION_SUITE_PROGRAMME_STORE_HPP
