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
  void notifyDuplicateScene(bool isDuplicate) {
    duplicateSceneDetected(isDuplicate);
  }
  void notifyExporting(bool isExporting) {
      exporting(isExporting);
  }
  void notifyProgrammeAdded(ProgrammeStatus status, proto::Programme const& programme) {
    programmeAdded(status, programme);
  }
  void notifyProgrammeRemoved(int programmeIndex) {
    programmeRemoved(programmeIndex);
  }
  void notifyProgrammeUpdated(ProgrammeStatus programmeIndex, proto::Programme const& programme) {
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
  void notifyInputUpdated(InputItem const& item, proto::InputItemMetadata const& oldItem) {
    inputUpdated(item, oldItem);
  }
 private:
  virtual void dataReset(proto::ProgrammeStore const& programmes, ItemMap const& items) {}
  virtual void duplicateSceneDetected(bool isDuplicate) {}
  virtual void exporting(bool isExporting) {}
  virtual void programmeAdded(ProgrammeStatus status, proto::Programme const& programme) {}
  virtual void programmeRemoved(int programmeIndex) {}
  virtual void programmeUpdated(ProgrammeStatus status, proto::Programme const& programme) {}
  virtual void programmeSelected(ProgrammeObjects const& objects) {}
  virtual void programmeMoved(Movement motion, proto::Programme const& programme) {}
  virtual void autoModeChanged(bool enabled) {}
  virtual void itemsAddedToProgramme(ProgrammeStatus status, std::vector<ProgrammeObject> const& objects) {}
  virtual void itemRemovedFromProgramme(ProgrammeStatus status, communication::ConnectionId const& id) {}
  virtual void programmeItemUpdated(ProgrammeStatus status, ProgrammeObject const& object) {}
  virtual void inputAdded(InputItem const& item) {}
  virtual void inputRemoved(communication::ConnectionId const& id) {}
  virtual void inputUpdated(InputItem const& item, proto::InputItemMetadata const& oldItem) {}
};

}  // namespace plugin
}  // namespace ear

#endif  // EAR_PRODUCTION_SUITE_METADATA_LISTENER_HPP
