#pragma once

#include "programme_store.pb.h"
#include "input_item_metadata.pb.h"
#include "store_metadata.hpp"
#include <adm/elements_fwd.hpp>
#include <bw64/bw64.hpp>
#include <vector>
#include <map>
#include <helper/common_definition_helper.h>

namespace ear {
namespace plugin {

class ProgrammeStoreAdmSerializer {
 public:
   struct PluginMap {
     uint32_t inputInstanceId;
     int32_t routing;
     std::shared_ptr<adm::AudioObject> audioObject;
     std::shared_ptr<adm::AudioTrackUid> audioTrackUid;
   };

  std::pair<std::shared_ptr<adm::Document>, std::vector<PluginMap>> serialize(
    std::pair<proto::ProgrammeStore, ItemMap> stores);
 private:
  void serializeToggle(std::shared_ptr<adm::AudioProgramme> programme,
                       const proto::Toggle& toggle);
  std::shared_ptr<adm::AudioObject> getToggleDefaultAudioObject(
      const std::shared_ptr<adm::AudioProgramme>& programme,
      const proto::Toggle& toggle);
  std::shared_ptr<adm::AudioObject> serializeToggleElement(std::shared_ptr<adm::AudioProgramme> programme,
                                                      const proto::Toggle& toggle,
                                                      int elementIndex);
  void serializeProgramme(adm::Document& doc,
                          proto::Programme const& programme);
  void serializeElement(adm::AudioContent& content,
                        proto::Object const& object);
  void createTopLevelObject(adm::AudioContent& content,
                            proto::InputItemMetadata const& metadata,
                            proto::Object const& object);
  bool isAlreadySerialized(proto::Object const& object) const;

  proto::ProgrammeStore programmes_;
  std::map<communication::ConnectionId, proto::InputItemMetadata> items_;
  std::shared_ptr<adm::Document> doc;
  std::vector<PluginMap> pluginMap;
  std::map<std::string, std::shared_ptr<adm::AudioObject>> serializedObjects;
  void setInteractivity(adm::AudioObject& object, const proto::Object& object1);
  void setImportance(adm::AudioObject& object, const proto::Object& object1);
  bool isSerializedWithDifferentObjectSettings(const proto::Object& object);

  AdmCommonDefinitionHelper admCommonDefinitionHelper;
};

}  // namespace plugin
}  // namespace ear
