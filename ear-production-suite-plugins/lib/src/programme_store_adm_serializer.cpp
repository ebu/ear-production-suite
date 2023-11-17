#include "programme_store_adm_serializer.hpp"
#include <speaker_setups.hpp>
#include <numeric>
#include <adm/adm.hpp>
#include <adm/utilities/object_creation.hpp>
#include <adm/common_definitions.hpp>
#include <helper/adm_preset_definitions_helper.h>
#include <utility>
#include <iomanip>
#include <optional>
#include <iostream>
#include <sstream>

using namespace ear::plugin;

namespace {

adm::AudioChannelFormatId genAudioChannelFormatId(int typedefinition, int value) {
  std::stringstream ss;
  ss << "AC_" 
    << std::setw(4) << std::setfill('0') << std::hex << typedefinition
    << std::setw(4) << std::setfill('0') << std::hex << value;
  return adm::parseAudioChannelFormatId(ss.str());
}

adm::AudioPackFormatId genAudioPackFormatId(int typedefinition,
                                                  int value) {
  std::stringstream ss;
  ss << "AP_" << std::setw(4) << std::setfill('0') << std::hex << typedefinition
     << std::setw(4) << std::setfill('0') << std::hex << value;
  return adm::parseAudioPackFormatId(ss.str());
}

template <typename T>
std::string int_to_hex(T i, size_t size) {
  std::stringstream stream;
  stream << std::setfill('0') << std::setw(size) << std::hex << i;
  return stream.str();
}

constexpr auto const ADM_DEFAULT_AZ = 0.0f;
constexpr auto const ADM_DEFAULT_EL = 0.0f;
// constexpr auto const ADM_DEFAULT_D = 0.f;
constexpr auto const ADM_DEFAULT_GAIN = 1.0f;

template <typename Param>
using ParameterValueT =
    std::remove_reference_t<decltype(std::declval<Param>().get())>;

template <typename Param, typename El>
ParameterValueT<Param> getValueOr(El const& element,
                                  ParameterValueT<Param> defaultValue) {
  if (element.template has<Param>()) return element.template get<Param>().get();
  return defaultValue;
}

bool approxEqual(float x, float y) { return fabs(x - y) < 0.001; }
float minAzimuth(ear::plugin::proto::PositionInteractive const& position) {
  return position.min_az();
}
float minAzimuth(adm::PositionInteractionRange const& position) {
  return getValueOr<adm::AzimuthInteractionMin>(position, ADM_DEFAULT_AZ);
}
float maxAzimuth(ear::plugin::proto::PositionInteractive const& position) {
  return position.min_az();
}
float maxAzimuth(adm::PositionInteractionRange const& position) {
  return getValueOr<adm::AzimuthInteractionMax>(position, ADM_DEFAULT_AZ);
}
float minElevation(proto::PositionInteractive const& position) {
  return position.min_el();
}
float minElevation(adm::PositionInteractionRange const& position) {
  return getValueOr<adm::ElevationInteractionMin>(position, ADM_DEFAULT_EL);
}
float maxElevation(proto::PositionInteractive const& position) {
  return position.max_el();
}
float maxElevation(adm::PositionInteractionRange const& position) {
  return getValueOr<adm::ElevationInteractionMax>(position, ADM_DEFAULT_EL);
}
// float minDistance(proto::PositionInteractive const& position) {
//  return position.min_r();
//}
// float minDistance(adm::PositionInteractionRange const& position) {
//  return getValueOr<adm::DistanceInteractionMin>(position, ADM_DEFAULT_D);
//}
// float maxDistance(proto::PositionInteractive const& position) {
//  return position.max_r();
//}
// float maxDistance(adm::PositionInteractionRange const& position) {
//  return getValueOr<adm::DistanceInteractionMax>(position, ADM_DEFAULT_D);
//}
bool operator==(adm::PositionInteractionRange const& lhs,
                proto::PositionInteractive const& rhs) {
  return approxEqual(maxElevation(lhs), maxElevation(rhs)) &&
         approxEqual(minElevation(lhs), minElevation(rhs)) &&
         //         approxEqual(maxDistance(lhs), maxDistance(rhs)) &&
         //         approxEqual(minDistance(lhs), minDistance(rhs)) &&
         approxEqual(maxAzimuth(lhs), maxAzimuth(rhs)) &&
         approxEqual(minAzimuth(lhs), minAzimuth(rhs));
}
bool operator==(proto::PositionInteractive const& lhs,
                adm::PositionInteractionRange const& rhs) {
  return rhs == lhs;
}
float minGain(proto::GainInteractive const& gain) { return gain.min(); }
float minGain(adm::GainInteractionRange const& gain) {
  return static_cast<float>(
      getValueOr<adm::GainInteractionMin>(gain,
                                          adm::Gain::fromLinear(ADM_DEFAULT_GAIN)).asLinear());
}
float maxGain(proto::GainInteractive const& gain) { return gain.max(); }
float maxGain(adm::GainInteractionRange const& gain) {
  return static_cast<float>(
      getValueOr<adm::GainInteractionMax>(gain,
                                          adm::Gain::fromLinear(ADM_DEFAULT_GAIN)).asLinear());
}
bool operator==(adm::GainInteractionRange const& lhs,
                proto::GainInteractive const& rhs) {
  return approxEqual(minGain(lhs), minGain(rhs)) &&
         approxEqual(maxGain(lhs), maxGain(rhs));
}
bool operator==(proto::GainInteractive const& lhs,
                adm::GainInteractionRange const& rhs) {
  return rhs == lhs;
}

struct InteractionStatus {
  bool onOffEnabled{false};
  bool gainEnabled{false};
  bool positionEnabled{false};
  bool enabled() { return onOffEnabled || gainEnabled || positionEnabled; }
};

InteractionStatus getInteractionStatus(proto::Object const& object) {
  return {object.has_interactive_on_off()
              ? object.interactive_on_off().enabled()
              : false,
          object.has_interactive_gain() ? object.interactive_gain().enabled()
                                        : false,
          object.has_interactive_position()
              ? object.interactive_position().enabled()
              : false};
}

bool interactionEqual(proto::Object const& object,
                      adm::AudioObject const& admObject) {
  auto interactions = getInteractionStatus(object);
  auto equal =
      (interactions.enabled() == getValueOr<adm::Interact>(admObject, false));
  auto admInteractionEnabled = admObject.has<adm::AudioObjectInteraction>();
  if (interactions.enabled()) {
    if (!admObject.has<adm::AudioObjectInteraction>()) {
      return false;
    }
    auto const& admInteraction = admObject.get<adm::AudioObjectInteraction>();
    equal = equal &&
            (interactions.onOffEnabled ==
             getValueOr<adm::OnOffInteract>(admInteraction, false)) &&
            (interactions.gainEnabled ==
             getValueOr<adm::GainInteract>(admInteraction, false)) &&
            (interactions.positionEnabled ==
             getValueOr<adm::PositionInteract>(admInteraction, false));
    if (equal) {
      if (interactions.gainEnabled) {
        auto gainRange = admInteraction.has<adm::GainInteractionRange>()
                             ? admInteraction.get<adm::GainInteractionRange>()
                             : adm::GainInteractionRange{};
        equal = (gainRange == object.interactive_gain());
      }
      if (interactions.positionEnabled) {
        auto positionRange =
            admInteraction.has<adm::PositionInteractionRange>()
                ? admInteraction.get<adm::PositionInteractionRange>()
                : adm::PositionInteractionRange{};
        equal = equal && positionRange == object.interactive_position();
      }
    }
    return equal;
  }
  return !admInteractionEnabled;
}

std::optional<int> determineImportanceValue(const proto::Object & object) {
  if(object.has_importance()) {
    return std::optional<int>(object.importance());
  }
#ifdef BAREBONESPROFILE
  return std::optional<int>(10);
#endif
  return std::optional<int>();
}

bool importanceEqual(proto::Object const& object,
                     adm::AudioObject const& admObject) {
  auto importance = determineImportanceValue(object);
  if(importance.has_value() == admObject.has<adm::Importance>()) {
    if(importance.has_value()) {
      auto admObjectImportance = admObject.get<adm::Importance>().get();
      return importance.value() == admObjectImportance;
    } else {
      return true;
    }
  }
  return false;
}

bool hasValidDefaultIndex(const proto::Toggle& toggle) {
  return toggle.has_default_element_index() &&
         toggle.default_element_index() >= 0 &&
         toggle.default_element_index() < toggle.element_size();
}

adm::SimpleObjectHolder add2076v2SimpleObjectTo(std::shared_ptr<adm::Document> document,
                                     const std::string& name) {
  // create
  adm::SimpleObjectHolder holder;
  holder.audioObject = adm::AudioObject::create(adm::AudioObjectName(name));

  holder.audioPackFormat = adm::AudioPackFormat::create(adm::AudioPackFormatName(name),
                                                        adm::TypeDefinition::OBJECTS);
  holder.audioChannelFormat = adm::AudioChannelFormat::create(
    adm::AudioChannelFormatName(name), adm::TypeDefinition::OBJECTS);
  holder.audioTrackUid = adm::AudioTrackUid::create();

  // reference
  holder.audioObject->addReference(holder.audioPackFormat);
  holder.audioPackFormat->addReference(holder.audioChannelFormat);
  holder.audioObject->addReference(holder.audioTrackUid);
  holder.audioTrackUid->setReference(holder.audioChannelFormat);
  holder.audioTrackUid->setReference(holder.audioPackFormat);

  document->add(holder.audioObject);
  return holder;
}

struct ObjectHolder {
  std::shared_ptr<adm::AudioObject> audioObject;
  std::vector<std::shared_ptr<adm::AudioTrackUid>> audioTrackUids;
};

ObjectHolder addPresetDefinitionObjectTo(
  std::shared_ptr<adm::Document> document, const std::string& name,
  const adm::AudioPackFormatId packFormatId,
  const std::vector<adm::AudioChannelFormatId>& channelFormatIds) {

  ObjectHolder holder;
  holder.audioObject = adm::AudioObject::create(adm::AudioObjectName(name));

  auto docPackFormat = document->lookup(packFormatId);
  if (!docPackFormat) {
    // not yet in document - shouldn't be a common def
    auto td = packFormatId.get<adm::TypeDescriptor>().get();
    auto pfIdVal = packFormatId.get<adm::AudioPackFormatIdValue>().get();
    auto presets = AdmPresetDefinitionsHelper::getSingleton();
    auto pfData = presets->getPackFormatData(td, pfIdVal);

    if (!pfData) {
      // not a preset (or therefore a common def)
      std::stringstream ss;
      ss << "AudioPackFormatId \"" << adm::formatId(packFormatId)
         << "\" not found in Preset Definitions. Can't author ADM.";
      throw adm::error::AdmException(ss.str());
    } 

    if (pfData->isCommonDefinition()) {
      // common def but not already in doc
      std::stringstream ss;
      ss << "AudioPackFormatId \"" << adm::formatId(packFormatId)
         << "\" not found in document. Are the common definitions added to the document?";
      throw adm::error::AdmException(ss.str());
    } 

    // It's a supplementary definition - add the necessary PF/CF tree
    docPackFormat = presets->copyMissingTreeElms(pfData->packFormat, document);
  }

  holder.audioObject->addReference(docPackFormat);
  holder.audioTrackUids.resize(channelFormatIds.size(), nullptr);
  for (size_t i = 0; i < channelFormatIds.size(); i++) {
    auto cf = document->lookup(channelFormatIds.at(i));
    if (!cf) {
      std::stringstream ss;
      ss << "AudioTrackFormatId \"" << adm::formatId(channelFormatIds.at(i))
        << "\" not found. Id might be invalid.";
      throw adm::error::AdmException(ss.str());
    }
    auto uid = adm::AudioTrackUid::create();
    uid->setReference(docPackFormat);
    uid->setReference(cf);
    holder.audioObject->addReference(uid);
    holder.audioTrackUids[i] = uid;
  }
  document->add(holder.audioObject);
  return holder;
}


}  // namespace

bool ProgrammeStoreAdmSerializer::isAlreadySerialized(
    const proto::Object& object) const {
  auto const& connectionId = object.connection_id();
  auto objectIt = serializedObjects.find(connectionId);
  if (objectIt != serializedObjects.end()) {
    auto const& admObject = *(objectIt->second);
    return importanceEqual(object, admObject) && interactionEqual(object, admObject);
  }
  return false;
}

std::pair<std::shared_ptr<adm::Document>, std::vector<ProgrammeStoreAdmSerializer::PluginMap>>
ProgrammeStoreAdmSerializer::serialize(std::pair<proto::ProgrammeStore, ItemMap> stores) {
  programmes_ = std::move(stores.first);
  items_ = std::move(stores.second);
  doc = adm::Document::create();
  doc->set(adm::Version("ITU-R_BS.2076-2"));
  addCommonDefinitionsTo(doc);
  pluginMap.clear();
  for (auto& programme : programmes_.programme()) {
    serializeProgramme(*doc, programme);
  }
  return {doc, pluginMap};
}

void ProgrammeStoreAdmSerializer::serializeToggle(
    std::shared_ptr<adm::AudioProgramme> programme,
    const proto::Toggle& toggle) {
  std::shared_ptr<adm::AudioObject> defaultObject =
      getToggleDefaultAudioObject(programme, toggle);
  if (defaultObject) {
    auto defaultIndex = toggle.default_element_index();
    for (auto i = 0; i != toggle.element_size(); ++i) {
      if (i != defaultIndex) {
        auto admObject = serializeToggleElement(programme, toggle, i);
        if (admObject) {
          defaultObject->addComplementary(admObject);
        }
      }
    }
  }
}

std::shared_ptr<adm::AudioObject>
ProgrammeStoreAdmSerializer::getToggleDefaultAudioObject(
    const std::shared_ptr<adm::AudioProgramme>& programme,
    const proto::Toggle& toggle) {
  if (hasValidDefaultIndex(toggle)) {
    return serializeToggleElement(programme, toggle,
                                  toggle.default_element_index());
  }
  return nullptr;
}

std::shared_ptr<adm::AudioObject>
ProgrammeStoreAdmSerializer::serializeToggleElement(
    std::shared_ptr<adm::AudioProgramme> programme, const proto::Toggle& toggle,
    int elementIndex) {
  auto const& element = toggle.element(elementIndex);
  if (element.has_object()) {
    auto const& object = element.object();
    if (object.has_connection_id()) {
      auto content = adm::AudioContent::create(adm::AudioContentName{""});
      programme->addReference(content);
      serializeElement(*content, object);
      auto admObject = serializedObjects[object.connection_id()];
      content->set(
          adm::AudioContentName{admObject->get<adm::AudioObjectName>().get()});
      return admObject;
    }
  }
  return nullptr;
}

void ProgrammeStoreAdmSerializer::serializeProgramme(
    adm::Document& doc, const proto::Programme& programme) {
  auto prog = adm::AudioProgramme::create(
      adm::AudioProgrammeName{programme.name()});
  if(programme.has_language() && !programme.language().empty()) {
    prog->set(adm::AudioProgrammeLanguage{ programme.language() });
  }
  auto defaultContent =
      adm::AudioContent::create(adm::AudioContentName{programme.name()});
  doc.add(prog);
  prog->addReference(defaultContent);
  for (auto const& element : programme.element()) {
    if (element.has_toggle()) {
      serializeToggle(prog, element.toggle());
    }
    if (element.has_group()) {
      // TODO group handling
    }
    if (element.has_object()) {
      serializeElement(*defaultContent, element.object());
    }
  }
}

void ProgrammeStoreAdmSerializer::serializeElement(
    adm::AudioContent& content, const proto::Object& object) {
  auto metaDataIt = items_.find(object.connection_id());
  if (metaDataIt != items_.end()) {
    if (metaDataIt->second.has_obj_metadata() ||
        metaDataIt->second.has_ds_metadata() ||
        metaDataIt->second.has_hoa_metadata()) {
      createTopLevelObject(content, metaDataIt->second, object);
    }
  }
}

void ProgrammeStoreAdmSerializer::createTopLevelObject(
    adm::AudioContent& content, const proto::InputItemMetadata& metadata,
    const proto::Object& object) {
  assert(metadata.has_obj_metadata() || metadata.has_ds_metadata() ||
         metadata.has_hoa_metadata());
  const auto& connectionId = metadata.connection_id();

  if (isAlreadySerialized(object)) {
    // Already have serialized this object (in a different programme?)
    content.addReference(serializedObjects[connectionId]);
  } else if (isSerializedWithDifferentObjectSettings(object)) {
    // Have serialized this input but have different object level settings
    auto otherAdmObject =
        serializedObjects.find(object.connection_id())->second;
    auto admObject =
        adm::AudioObject::create(adm::AudioObjectName(metadata.name()));
    content.addReference(admObject);
    for (auto& atuid : otherAdmObject->getReferences<adm::AudioTrackUid>()) {
      admObject->addReference(atuid);
    }
    for (auto& pack : otherAdmObject->getReferences<adm::AudioPackFormat>()) {
      admObject->addReference(pack);
    }
    setInteractivity(*admObject, object);
    setImportance(*admObject, object);

    // Create plugin map entries for new AudioObject (copy other params from existing)
    int existingPluginMapEntryCount = pluginMap.size();
    for(int i = 0; i < existingPluginMapEntryCount; ++i) {
      if(pluginMap[i].audioObject == otherAdmObject) {
        pluginMap.push_back({ pluginMap[i].inputInstanceId, pluginMap[i].routing, admObject, pluginMap[i].audioTrackUid });
      }
    }

  } else {
    // First time we've seen the input
    if (metadata.has_obj_metadata()) {
      auto objectHolder = add2076v2SimpleObjectTo(doc, metadata.name());
      setInteractivity(*objectHolder.audioObject, object);
      setImportance(*objectHolder.audioObject, object);
      content.addReference(objectHolder.audioObject);
      assert(metadata.routing() <= std::numeric_limits<int16_t>::max());
      serializedObjects[connectionId] = objectHolder.audioObject;
      pluginMap.push_back({ metadata.input_instance_id(), metadata.routing(), objectHolder.audioObject, objectHolder.audioTrackUid });

    } 
    else if (metadata.has_hoa_metadata()) {
      // get the pack format Id from the metadata and from that create the full
      // pack format string This can be used to look up channel information using
      // the common definitions helper
      int packFormatIdValue = metadata.hoa_metadata().packformatidvalue();
      int packFormatId = 0x00040000 | packFormatIdValue;
      std::string packFormatIdStr =
          std::string("AP_") + int_to_hex(packFormatId, 8);
      auto packFormat =
          doc->lookup(adm::parseAudioPackFormatId(packFormatIdStr));

      if(packFormat) {
        auto hoaAudioObject =
          adm::AudioObject::create(adm::AudioObjectName(metadata.name()));
        hoaAudioObject->addReference(packFormat);
        content.addReference(hoaAudioObject);
        setInteractivity(*hoaAudioObject, object);
        setImportance(*hoaAudioObject, object);

        auto channelFormats =
          AdmPresetDefinitionsHelper::getSingleton()
          ->getPackFormatData(4, packFormatIdValue)
          ->relatedChannelFormats;
        // The AudioTrackIUD needs to reference the track format
        // To get the correct track format we go through all track formats and
        // find the one that references the correct channel format
        auto allTrackFormats = doc->getElements<adm::AudioTrackFormat>();

        // For each channel format create a AudioTrackUid that references the pack
        // format. The audio object also needs to reference the channel format.
        for(int i(0); i < channelFormats.size(); ++i) {
          auto audioTrackUid = adm::AudioTrackUid::create();
          hoaAudioObject->addReference(audioTrackUid);
          audioTrackUid->setReference(packFormat);

          // We need the channel format from our document,
          // not the document AdmPresetDefinitionsHelper is using!
          auto channelFormatId = adm::AudioChannelFormatId(
            adm::TypeDefinition::HOA,
            adm::AudioChannelFormatIdValue(channelFormats[i]->idValue));
          auto channelFormat = doc->lookup(channelFormatId);
          assert(channelFormat);

          audioTrackUid->setReference(channelFormat);
          pluginMap.push_back({ metadata.input_instance_id(), metadata.routing() + i, hoaAudioObject, audioTrackUid });
        }
        serializedObjects[connectionId] = hoaAudioObject;
      }

    } 
    else if (metadata.has_ds_metadata()) {
      std::vector<adm::AudioChannelFormatId> channelFormatIds;

      for (auto& speaker : metadata.ds_metadata().speakers()) {
        auto channelFormatId = genAudioChannelFormatId(1, speaker.id());
        channelFormatIds.push_back(channelFormatId);
      }
      auto objectHolder = addPresetDefinitionObjectTo(
          doc, metadata.name(),
          genAudioPackFormatId(1, metadata.ds_metadata().packformatidvalue()),
          channelFormatIds);

      setInteractivity(*objectHolder.audioObject, object);
      setImportance(*objectHolder.audioObject, object);
      content.addReference(objectHolder.audioObject);
      for (int i(0); i < objectHolder.audioTrackUids.size(); ++i) {
        pluginMap.push_back({ metadata.input_instance_id(), metadata.routing() + i, objectHolder.audioObject, objectHolder.audioTrackUids[i] });
      }
      serializedObjects[connectionId] = objectHolder.audioObject;
    }
  }
}

void ProgrammeStoreAdmSerializer::setInteractivity(
    adm::AudioObject& admObject, const proto::Object& object) {
  auto interactive = getInteractionStatus(object);
  if (interactive.enabled()) {
    admObject.set(adm::Interact{true});
    auto interaction = adm::AudioObjectInteraction{
        adm::OnOffInteract{interactive.onOffEnabled}};
    if (interactive.gainEnabled) {
      interaction.set(adm::GainInteract{true});
      auto const& interactiveGain = object.interactive_gain();
      auto gainRange = adm::GainInteractionRange{
          adm::GainInteractionMin{adm::Gain::fromLinear(interactiveGain.min())},
          adm::GainInteractionMax{adm::Gain::fromLinear(interactiveGain.max())}};
      interaction.set(gainRange);
    }
    if (interactive.positionEnabled) {
      interaction.set(adm::PositionInteract{true});
      auto const& interactivePosition = object.interactive_position();
      auto positionRange = adm::PositionInteractionRange{
          adm::AzimuthInteractionMin{interactivePosition.min_az()},
          adm::AzimuthInteractionMax{interactivePosition.max_az()},
          //          adm::DistanceInteractionMin{interactivePosition.min_r()},
          //          adm::DistanceInteractionMax{interactivePosition.max_r()},
          adm::ElevationInteractionMin{interactivePosition.min_el()},
          adm::ElevationInteractionMax{interactivePosition.max_el()}};
      interaction.set(positionRange);
    }
    admObject.set(interaction);
  }
}

void ear::plugin::ProgrammeStoreAdmSerializer::setImportance(adm::AudioObject & admObject, const proto::Object & object)
{
  auto importance = determineImportanceValue(object);
  if(importance.has_value()) {
    admObject.set(adm::Importance(importance.value()));
  } else {
    admObject.unset<adm::Importance>();
  }
}

bool ProgrammeStoreAdmSerializer::isSerializedWithDifferentObjectSettings(
    const proto::Object& object) {
  auto const& connectionId = object.connection_id();
  auto objectIt = serializedObjects.find(connectionId);
  return objectIt != serializedObjects.end();
}
