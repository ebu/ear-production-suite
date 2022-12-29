#include "programme_store_adm_populator.hpp"
#include "programme_internal_id.hpp"
#include "helper/container_helpers.hpp"

using namespace ear::plugin;
using namespace adm;
namespace {

constexpr auto const ADM_DEFAULT_AZ = 0.0f;
constexpr auto const ADM_DEFAULT_EL = 0.0f;
constexpr auto const ADM_DEFAULT_D = 0.f;
constexpr auto const ADM_DEFAULT_GAIN = 1.0f;

bool idValueEquals(uint32_t const& idAsUint,
                   std::shared_ptr<const AudioTrackUid> id) {
  auto idVal =
      id->get<adm::AudioTrackUidId>().get<adm::AudioTrackUidIdValue>().get();
  return idVal == idAsUint;
}

template <typename PropertyT, typename ElementT>
bool isPresentAndEnabled(ElementT const& element) {
  return element.template has<PropertyT>() &&
         element.template get<PropertyT>().get();
}

void setGainInteraction(proto::Object* object,
                        const AudioObjectInteraction& admInteraction);
void setPositionInteraction(proto::Object* object,
                            const AudioObjectInteraction& admInteraction);
void setOnOffInteraction(proto::Object* object,
                         const AudioObjectInteraction& admInteraction);

void setImportance(proto::Object* object,
                      adm::AudioObject const& admObject) {
  if(admObject.has<adm::Importance>()) {
    object->set_importance(admObject.get<adm::Importance>().get());
  } else {
    object->clear_importance();
  }
}

void setInteractivity(proto::Object* object,
                      adm::AudioObject const& admObject) {
  if (admObject.has<adm::Interact>() && admObject.get<adm::Interact>().get()) {
    if (admObject.has<adm::AudioObjectInteraction>()) {
      auto admInteraction = admObject.get<adm::AudioObjectInteraction>();
      setOnOffInteraction(object, admInteraction);
      setGainInteraction(object, admInteraction);
      setPositionInteraction(object, admInteraction);
    }
  }
}

void setOnOffInteraction(proto::Object* object,
                         const AudioObjectInteraction& admInteraction) {
  if (isPresentAndEnabled<OnOffInteract>(admInteraction)) {
    object->mutable_interactive_on_off()->set_enabled(true);
  }
}

void setGainInteraction(proto::Object* object,
                        const AudioObjectInteraction& admInteraction) {
  if (isPresentAndEnabled<GainInteract>(admInteraction)) {
    auto interactiveGain = object->mutable_interactive_gain();
    interactiveGain->set_enabled(true);
    if (admInteraction.has<GainInteractionRange>()) {
      auto gainRange = admInteraction.get<GainInteractionRange>();
      if (gainRange.has<GainInteractionMin>()) {
        interactiveGain->set_min(gainRange.get<GainInteractionMin>().get().asLinear());
      } else {
        interactiveGain->set_min(ADM_DEFAULT_GAIN);
      }
      if (gainRange.has<GainInteractionMax>()) {
        interactiveGain->set_max(gainRange.get<GainInteractionMax>().get().asLinear());
      } else {
        interactiveGain->set_max(ADM_DEFAULT_GAIN);
      }
    }
  }
}

void setPositionInteraction(proto::Object* object,
                            const AudioObjectInteraction& admInteraction) {
  if (isPresentAndEnabled<PositionInteract>(admInteraction)) {
    auto interactivePosition = object->mutable_interactive_position();
    interactivePosition->set_enabled(true);
    if (admInteraction.has<PositionInteractionRange>()) {
      auto positionRange = admInteraction.get<PositionInteractionRange>();
      if (positionRange.has<AzimuthInteractionMin>()) {
        interactivePosition->set_min_az(
            positionRange.get<AzimuthInteractionMin>().get());
      } else {
        interactivePosition->set_min_az(ADM_DEFAULT_AZ);
      }
      if (positionRange.has<ElevationInteractionMin>()) {
        interactivePosition->set_min_el(
            positionRange.get<ElevationInteractionMin>().get());
      } else {
        interactivePosition->set_min_el(ADM_DEFAULT_EL);
      }
      if (positionRange.has<DistanceInteractionMin>()) {
        interactivePosition->set_min_r(
            positionRange.get<DistanceInteractionMin>().get());
      } else {
        interactivePosition->set_min_r(ADM_DEFAULT_D);
      }
      if (positionRange.has<AzimuthInteractionMax>()) {
        interactivePosition->set_max_az(
            positionRange.get<AzimuthInteractionMax>().get());
      } else {
        interactivePosition->set_max_az(ADM_DEFAULT_AZ);
      }
      if (positionRange.has<ElevationInteractionMax>()) {
        interactivePosition->set_max_el(
            positionRange.get<ElevationInteractionMax>().get());
      } else {
        interactivePosition->set_max_el(ADM_DEFAULT_EL);
      }
      if (positionRange.has<DistanceInteractionMax>()) {
        interactivePosition->set_max_r(
            positionRange.get<DistanceInteractionMax>().get());
      } else {
        interactivePosition->set_max_r(ADM_DEFAULT_D);
      }
    }
  }
}
}  // namespace

void ProgrammeStoreAdmPopulator::operator()(
    std::shared_ptr<const AudioProgramme> prog) {
  proto::Programme* protoProg{nullptr};
  auto insertionIt = importedProgrammes.end();
  bool success{false};
  std::tie(insertionIt, success) =
      importedProgrammes.insert(make_pair(prog, protoProg));
  if (success) {
    auto programmeElement = store->add_programme();
    if (prog->has<adm::AudioProgrammeName>()) {
      programmeElement->set_name(prog->get<adm::AudioProgrammeName>().get());
    }
    if (prog->has<adm::AudioProgrammeLanguage>()) {
      programmeElement->set_language(
          prog->get<adm::AudioProgrammeLanguage>().get());
    }
    auto id = newProgrammeInternalId();
    programmeElement->set_programme_internal_id(id);
    insertionIt->second = programmeElement;
  }
  currentProgramme = insertionIt->second;

  if (store->programme_size() == 1) {
    store->set_selected_programme_internal_id(store->programme(0).programme_internal_id());
  }
}

void ProgrammeStoreAdmPopulator::operator()(
  std::shared_ptr<const AudioObject> admObject) {

  // Make sure it's a bottom-level audioObject
  auto pfs = admObject->getReferences<adm::AudioPackFormat>();
  if(pfs.size() != 0) {

    auto aoId = admObject->get<adm::AudioObjectId>().get<adm::AudioObjectIdValue>().get();
    auto ids = std::make_pair(admObject->get<adm::AudioObjectId>(),
                              adm::AudioTrackUidId(adm::AudioTrackUidIdValue(0)));

    auto td = pfs.front()->get<adm::TypeDescriptor>();
    if(td == adm::TypeDefinition::OBJECTS) {
      // Expect one plugin per trackuid
      auto atus = admObject->getReferences<adm::AudioTrackUid>();
      for(auto const& atu : atus) {
        ids.second = atu->get<adm::AudioTrackUidId>();
        auto found = programmeElementImportedIdsLookup.find(ids) != programmeElementImportedIdsLookup.end();
        if(!found) {
          auto element = currentProgramme->add_element();
          setInteractivity(element->mutable_object(), *admObject);
          setImportance(element->mutable_object(), *admObject);
          programmeElementImportedIdsLookup[ids] = element;
        }
      }

    } else {
      // Expect one plugin for all trackuids
      auto found = programmeElementImportedIdsLookup.find(ids) != programmeElementImportedIdsLookup.end();
      if(!found) {
        auto element = currentProgramme->add_element();
          setInteractivity(element->mutable_object(), *admObject);
          programmeElementImportedIdsLookup[ids] = element;
      }
    }
  }

}

void ProgrammeStoreAdmPopulator::operator()(
    std::shared_ptr<const AudioTrackUid> uid) {
}

void ProgrammeStoreAdmPopulator::operator()(
    std::shared_ptr<const AudioChannelFormat> channel) {
}

void ProgrammeStoreAdmPopulator::startRoute() {
  programmeElementImportedIdsLookup.clear();
}

void ProgrammeStoreAdmPopulator::endRoute() {
  elementImportedIdsLookup.insert(programmeElementImportedIdsLookup.begin(),
                            programmeElementImportedIdsLookup.end());
}

std::multimap<std::pair<adm::AudioObjectId, adm::AudioTrackUidId>, proto::ProgrammeElement*> ear::plugin::populateStoreFromAdm(
    const Document& doc, proto::ProgrammeStore& store) {
  auto tracer = adm::detail::GenericRouteTracer<adm::Route, StopAtChannel>{};
  store = proto::ProgrammeStore{};
  ProgrammeStoreAdmPopulator populator(&store);

  for (auto& programme : doc.getElements<adm::AudioProgramme>()) {
    populator.startRoute();
    auto routes = tracer.run(programme);
    for (auto const& route : routes) {
      for (auto& element : route) {
        boost::apply_visitor(populator, element);
      }
    }
    populator.endRoute();
  }

  // for adm files with no high level metadata (no programmes)
  // just stay in auto mode... otherwise...
  if (store.programme_size() > 0) {
    store.set_auto_mode(false);

    store.set_selected_programme_internal_id(store.programme(0).programme_internal_id());
    return populator.getImportedIdsLookup();
  }
  return {};
}
