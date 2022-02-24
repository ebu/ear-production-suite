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
#include "programme_store.pb.h"

namespace ear::plugin {
class ItemStore;
namespace communication {
class ConnectionId;
}

class ProgrammeStoreListener {
 public:
  void objectAdded(proto::Object const& object) {
    addObject(object);
  }
  void storeChanged(proto::ProgrammeStore const& store) {
    changeStore(store);
  }

 protected:
  virtual void addObject(proto::Object const& object) {}
  virtual void changeStore(proto::ProgrammeStore const& store) {}

};

class ProgrammeStore {
 public:
  [[ nodiscard ]] proto::ProgrammeStore get() const;
  [[ nodiscard ]] std::optional<proto::Programme> selectedProgramme() const;
  [[ nodiscard ]] std::optional<proto::Programme> programmeAtIndex(int index) const;
  [[ nodiscard ]] int programmeCount() const;
  [[ nodiscard ]] std::string programmeName(int index) const;;
  [[ nodiscard ]] bool autoModeEnabled() const;

  void set(proto::ProgrammeStore store);
  void setAutoMode(bool enable);
  void selectProgramme(int index);
  std::pair<int, std::string> addProgramme();
  bool moveProgramme(int oldIndex, int newIndex);
  void removeProgramme(int index);
  void setProgrammeName(int index, std::string const& name);
  void setProgrammeLanguage(int programmeIndex, std::string const& language);
  void clearProgrammeLanguage(int programmeIndex);
  std::pair<int, proto::Object> addItemToSelectedProgramme(communication::ConnectionId const& id);
  void removeElement(int programmeIndex, int elementIndex);
  bool updateElement(communication::ConnectionId const& id, proto::Object const& element);
  std::vector<int> removeFromAllProgrammes(communication::ConnectionId const& id);
  bool moveElement(int programmeIndex, int oldIndex, int newIndex);
  void autoUpdateFrom(ItemStore const& itemStore);

  void addListener(std::shared_ptr<ProgrammeStoreListener> const& listener);

 private:
  std::optional<proto::Programme> programmeAtIndexImpl(int index) const;
  proto::Programme* addProgrammeImpl(std::string const& name, std::string const& language);
  proto::Object* addObject(
      proto::Programme* programme,
      const communication::ConnectionId id);


  void notifyObjectAdded(proto::Object const& object);
  void notifyStoreChanged(proto::ProgrammeStore const& store);

  std::vector<std::weak_ptr<ProgrammeStoreListener>> listeners_;
  proto::ProgrammeStore store_;
  mutable std::mutex mutex_;
};

[[ nodiscard ]]
bool isItemInProgramme(communication::ConnectionId const& id,
                       proto::Programme const& programme);
}

#endif  // EAR_PRODUCTION_SUITE_PROGRAMME_STORE_HPP
