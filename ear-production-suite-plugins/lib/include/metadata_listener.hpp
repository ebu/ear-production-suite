//
// Created by Richard Bailey on 09/03/2022.
//

#ifndef EAR_PRODUCTION_SUITE_METADATA_LISTENER_HPP
#define EAR_PRODUCTION_SUITE_METADATA_LISTENER_HPP
#include "metadata.hpp"

namespace ear {
namespace plugin {


class MetadataListener {
 public:
  virtual ~MetadataListener() = default;

  void notifyDataReset(proto::ProgrammeStore const& programmes, ItemMap const& items) {
    dataReset(programmes, items);
  }
  void notifyProgrammeAdded(int programmeIndex, proto::Programme const& programme) {
    programmeAdded(programmeIndex, programme);
  }
  void notifyProgrammeRemoved(int programmeIndex) {
    programmeRemoved(programmeIndex);
  }
  void notifyProgrammeUpdated(int programmeIndex, proto::Programme const& programme) {
    programmeUpdated(programmeIndex, programme);
  }
  void notifyProgrammeSelected(ProgrammeObjects const& objects) {
    programmeSelected(objects);
  }
  void notifyProgrammeMoved(Movement motion, proto::Programme const& programme) {
    programmeMoved(motion, programme);
  }
  void notifyAutoModeChanged(bool enabled) {
    autoModeChanged(enabled);
  }
  void notifyItemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& objects) {
    itemsAddedToProgramme(status, objects);
  }
  void notifyItemRemovedFromProgramme(ProgrammeStatus status, communication::ConnectionId const& id) {
    itemRemovedFromProgramme(status, id);
  }
  void notifyProgrammeItemUpdated(ProgrammeStatus status, ProgrammeObject const& object) {
    programmeItemUpdated(status, object);
  }
  void notifyInputAdded(InputItem const& item) {
    inputAdded(item);
  }
  void notifyInputRemoved(communication::ConnectionId const& id) {
    inputRemoved(id);
  }
  void notifyInputUpdated(InputItem const& item) {
    inputUpdated(item);
  }
 private:
  virtual void dataReset(proto::ProgrammeStore const& programmes, ItemMap const& items) {}
  virtual void programmeAdded(int programmeIndex, proto::Programme const& programme) {}
  virtual void programmeRemoved(int programmeIndex) {}
  virtual void programmeUpdated(int programmeIndex, proto::Programme const& programme) {}
  virtual void programmeSelected(ProgrammeObjects const& objects) {}
  virtual void programmeMoved(Movement motion, proto::Programme const& programme) {}
  virtual void autoModeChanged(bool enabled) {}
  virtual void itemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& objects) {}
  virtual void itemRemovedFromProgramme(ProgrammeStatus status, communication::ConnectionId const& id) {}
  virtual void programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& object) {}
  virtual void inputAdded(InputItem const& item) {}
  virtual void inputRemoved(communication::ConnectionId const& id) {}
  virtual void inputUpdated(InputItem const& item) {}
};

}  // namespace plugin
}  // namespace ear

#endif  // EAR_PRODUCTION_SUITE_METADATA_LISTENER_HPP