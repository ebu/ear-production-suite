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

struct ProgrammeStatus {
  int index;
  bool isSelected;
};

struct Movement {
  int from;
  int to;
};

class ProgrammeStoreListener {
 public:
  void itemsAdded(ProgrammeStatus status, std::vector<proto::Object> const& items) {
    addItems(status, items);
  }
  void itemRemoved(ProgrammeStatus status, proto::Object const& item) {
    removeItem(status, item);
  }
  void itemUpdated(ProgrammeStatus status, proto::Object const& item) {
    updateItem(status, item);
  }
  void storeChanged(proto::ProgrammeStore const& store) {
    changeStore(store);
  }
  void programmeAdded(ProgrammeStatus status, proto::Programme const& programme) {
    addProgramme(status, programme);
  }

  void programmeMoved(Movement movement, proto::Programme const& programme) {
    moveProgramme(movement, programme);
  }

  void programmeSelected(int index, proto::Programme const& programme) {
    selectProgramme(index, programme);
  }

  void programmeRemoved(int index) {
    removeProgramme(index);
  }

  void autoModeSet(bool enabled) {
    setAutoMode(enabled);
  }

  void programmeUpdated(int index, proto::Programme const& programme) {
    updateProgramme(index, programme);
  }

 protected:
  virtual void addItems(ProgrammeStatus status, std::vector<proto::Object> const& items) {}
  virtual void removeItem(ProgrammeStatus status, proto::Object const& item) {}
  virtual void updateItem(ProgrammeStatus status, proto::Object const& item) {}
  virtual void addProgramme(ProgrammeStatus status, proto::Programme const& programme) {}
  virtual void changeStore(proto::ProgrammeStore const& store) {}
  virtual void moveProgramme(Movement movement, proto::Programme const& programme) {}
  virtual void removeProgramme(int index) {}
  virtual void selectProgramme(int index, proto::Programme const& programme) {}
  virtual void setAutoMode(bool enabled) {}
  virtual void updateProgramme(int index, proto::Programme const& programme) {}
};

class ProgrammeStore {
 public:
  [[ nodiscard ]] proto::ProgrammeStore get() const;
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
  void removeElementFromProgramme(int programmeIndex, int elementIndex);
  void removeElementFromAllProgrammes(communication::ConnectionId const& id);
  void moveElement(int programmeIndex, int oldIndex, int newIndex);
  void autoUpdateFrom(ItemStore const& itemStore);
  void addListener(std::shared_ptr<ProgrammeStoreListener> const& listener);

 private:
  void removeElementFromProgramme(int programmeIndex, communication::ConnectionId const& id);
  std::optional<proto::Programme> programmeAtIndexImpl(int index) const;
  proto::Programme* addProgrammeImpl(std::string const& name, std::string const& language);
  proto::Object* addObject(
      proto::Programme* programme,
      const communication::ConnectionId id);

  template <typename Fn>
  void fireEvent(Fn&& fn) {
    removeDeadPointersAfter(listeners_,
                            std::forward<Fn>(fn));
  }

  std::vector<std::weak_ptr<ProgrammeStoreListener>> listeners_;
  proto::ProgrammeStore store_;
};

[[ nodiscard ]]
bool isItemInProgramme(communication::ConnectionId const& id,
                       proto::Programme const& programme);
}

#endif  // EAR_PRODUCTION_SUITE_PROGRAMME_STORE_HPP
