#include "ui/scene_frontend_backend_connector.hpp"

namespace ear {
namespace plugin {
namespace ui {

//void SceneFrontendBackendConnector::addItem(communication::ConnectionId id) {
//  doAddItem(id);
//}
//
//void SceneFrontendBackendConnector::removeInput(communication::ConnectionId id) {
//  doRemoveItem(id);
//}
//
//void SceneFrontendBackendConnector::updateItem(communication::ConnectionId id,
//                                               proto::InputItemMetadata item) {
//  doUpdateItem(id, item);
//}

void SceneFrontendBackendConnector::notifyProgrammeStoreChanged(
    proto::ProgrammeStore store) {
  if (programmeStoreCallback_) {
    programmeStoreCallback_(std::move(store));
  }
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
