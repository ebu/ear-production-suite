#include "programme_store_adm_populator.hpp"
using namespace ear::plugin;
using namespace adm;
namespace {

constexpr auto const ADM_DEFAULT_AZ = 0.0f;
constexpr auto const ADM_DEFAULT_EL = 0.0f;
constexpr auto const ADM_DEFAULT_D = 0.f;
constexpr auto const ADM_DEFAULT_GAIN = 1.0f;

template <typename C, typename T>
bool contains(C const& container, T const& element) {
  return std::find(container.begin(), container.end(), element) !=
         container.end();
}

template <typename C, typename T>
auto find(C& container, T& element) {
  return std::find(container.begin(), container.end(), element);
}

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
    insertionIt->second = programmeElement;
  }
  currentProgramme = insertionIt->second;

  if (store->programme_size() == 1) {
    store->set_selected_programme_index(0);
  }
}

void ProgrammeStoreAdmPopulator::operator()(
    std::shared_ptr<const AudioObject> admObject) {
  currentObject = admObject;
  auto currentUids = admObject->getReferences<AudioTrackUid>();
  auto it =
      std::find_first_of(trackMaps.begin(), trackMaps.end(),
                         currentUids.begin(), currentUids.end(), idValueEquals);
  if (it != trackMaps.end()) {
    auto packs = admObject->getReferences<adm::AudioPackFormat>();
    if (!packs.empty() && packs.front()->get<TypeDescriptor>() ==
                              TypeDefinition::DIRECT_SPEAKERS) {
      auto location = std::distance(trackMaps.begin(), it);
      auto found = programmeElementTrackLookup.find(location);
      if(found == programmeElementTrackLookup.end()) {
        auto element = currentProgramme->add_element();
        setInteractivity(element->mutable_object(), *admObject);
        programmeElementTrackLookup[location] = element;
      }
    }
  }
}

void ProgrammeStoreAdmPopulator::operator()(
    std::shared_ptr<const AudioTrackUid> uid) {
  currentUid = uid;
}

void ProgrammeStoreAdmPopulator::operator()(
    std::shared_ptr<const AudioChannelFormat> channel) {
  if (channel->get<TypeDescriptor>() == TypeDefinition::OBJECTS) {
    auto uid = currentUid;
    auto it = std::find_if(
        trackMaps.begin(), trackMaps.end(),
        [uid](uint32_t value) { return idValueEquals(value, uid); });
    if (it != trackMaps.end()) {
      auto location = std::distance(trackMaps.begin(), it);
      auto element = currentProgramme->add_element();
      setInteractivity(element->mutable_object(), *currentObject);
      programmeElementTrackLookup[location] = element;
    }
  }
}

void ProgrammeStoreAdmPopulator::startRoute() {
  programmeElementTrackLookup.clear();
}

void ProgrammeStoreAdmPopulator::endRoute() {
  elementTrackLookup.insert(programmeElementTrackLookup.begin(),
                            programmeElementTrackLookup.end());
}

std::multimap<int, proto::ProgrammeElement*> ear::plugin::populateStoreFromAdm(
    const Document& doc, proto::ProgrammeStore& store,
    const std::vector<uint32_t>& trackMaps) {
  // This should only result in prog->content->obj->uid->pack routes
  // i.e. nested objects ignored
  auto tracer = adm::detail::GenericRouteTracer<adm::Route, StopAtChannel>{};
  store = proto::ProgrammeStore{};
  ProgrammeStoreAdmPopulator populator(&store, trackMaps);

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
  // just stay in auto mode
  if (store.programme_size() > 0) {
    store.set_auto_mode(false);
    store.set_selected_programme_index(0);
    return populator.getTrackLookup();
  }
  return {};
}
