#include <catch2/catch_all.hpp>
#include <sstream>
#include <programme_store.pb.h>
#include <scene_store.pb.h>
#include <programme_store_adm_serializer.hpp>
#include <adm/common_definitions.hpp>
#include <algorithm>
#include <functional>
#include "store_metadata.hpp"

using namespace ear::plugin;
using namespace ear::plugin::communication;
using namespace ear::plugin::proto;
using namespace std::placeholders;

namespace {

template<typename T>
int numberOf(adm::Document const& doc) {
  return doc.getElements<T>().size();
}

template<typename T>
int numberOfUncommon(adm::Document const& doc) {
  auto static commonDoc = adm::getCommonDefinitions();
  return numberOf<T>(doc) - numberOf<T>(*commonDoc);
}

class ItemBuilder {
  public:
    operator InputItemMetadata() const& {
      return metadata_;
    }
    operator InputItemMetadata() && {
      return std::move(metadata_);
    }
    ItemBuilder& withConnectionId(const std::string& id) {
      metadata_.set_connection_id(id);
      return *this;
    }
    ItemBuilder& withConnectionId(ConnectionId const& id) {
      metadata_.set_connection_id(id.string());
      return *this;
    }
    ItemBuilder& withRouting(int routing) {
      metadata_.set_routing(routing);
      return *this;
    }
    ItemBuilder& withName(const std::string& name) {
      metadata_.set_name(name);
      return *this;
    }
    ItemBuilder& withColour(int colour) {
      metadata_.set_colour(colour);
      return *this;
    }
    ItemBuilder& withContinuityCounter(int counter) {
      metadata_.set_continuity_counter(counter);
      return *this;
    }
    ItemBuilder& withChanged(bool changed) {
      metadata_.set_changed(changed);
      return *this;
    }
    ItemBuilder& withAllocatedObjMetadata(ObjectsTypeMetadata* objData) {
      metadata_.set_allocated_obj_metadata(objData);
      return *this;
    }
    ItemBuilder& withDefaultObjMetadata() {
      metadata_.mutable_obj_metadata();
      return *this;
    }
    ItemBuilder& withDefaultDsMetadata() {
      metadata_.mutable_ds_metadata();
      return *this;
    }
    ItemBuilder& withDsLayout(proto::SpeakerLayout layout) {
      auto dsMeta = metadata_.mutable_ds_metadata();
      dsMeta->set_layout(layout);
      return *this;
    }
    InputItemMetadata build() const& {
      return metadata_;
    }
    InputItemMetadata build() && {
      return std::move(metadata_);
    }
    InputItemMetadata metadata_;
};

class ObjectBuilder {
 public:
  operator Object() const& {
    return metadata_;
  }
  operator Object() && {
    return std::move(metadata_);
  };
  ObjectBuilder& withItem(ConnectionId const& id) {
    return withItem(id.string());
  }
  ObjectBuilder& withItem(std::string const& id) {
    metadata_.set_connection_id(id);
    return *this;
  }
  ObjectBuilder& withOnOffInteractionEnabled() {
    metadata_.mutable_interactive_on_off()->set_enabled(true);
    return *this;
  }
 private:
  Object metadata_;
};

class ToggleBuilder {
 public:
  operator Toggle() const& {
    return metadata_;
  }
  operator Toggle() && {
    return std::move(metadata_);
  }
  ToggleBuilder& withItem(ConnectionId const& id) {
    return withItem(id, false);
  }
  ToggleBuilder& withDefaultItem(ConnectionId const& id) {
    return withItem(id, true);
  }
  ToggleBuilder& with(Object const& object) {
    auto element = metadata_.add_element();
    *element->mutable_object() = object;
    return *this;
  }
 private:
  ToggleBuilder& withItem(ConnectionId const& id, bool setDefault) {
    auto element = metadata_.add_element();
    element->mutable_object()->set_connection_id(id.string());
    if(setDefault) {
      metadata_.set_default_element_index(metadata_.element_size() - 1);
    }
    return *this;
  }
  Toggle metadata_;
};

class ProgrammeBuilder {
 public:
  ProgrammeBuilder& withObject(ConnectionId id) {
    return with(ObjectBuilder{}.withItem(id));
  }
  ProgrammeBuilder& with(Object const& object) {
    auto element = programme_.add_element();
    *element->mutable_object() = object;
    return *this;
  }
  ProgrammeBuilder& withToggle(Toggle const& toggle) {
    auto progItem = programme_.add_element();
    auto progToggle = progItem->mutable_toggle();
    *progToggle = toggle;
    return *this;
  }
  ProgrammeBuilder& withName(const std::string& name) {
    programme_.set_name(name);
    return *this;
  }
  ProgrammeBuilder& withLanguage(const std::string& name) {
    programme_.set_language(name);
    return *this;
  }
  void addToStore(proto::ProgrammeStore& store) const {
    auto prog = store.add_programme();
    *prog = programme_;
  }
  Programme programme_;
};

class StoreBuilder {
 public:
  operator proto::ProgrammeStore() const& {
    return store_;
  }
  operator proto::ProgrammeStore() && {
    return std::move(store_);
  }
  proto::ProgrammeStore build() const& {
    return store_;
  }
  proto::ProgrammeStore build() && {
    return std::move(store_);
  }
  StoreBuilder& withProgramme(ProgrammeBuilder& builder) {
    builder.addToStore(store_);
    return *this;
  }
 private:
  proto::ProgrammeStore store_;
};

ItemBuilder simpleItem(ConnectionId const& id,
                       std::string const& name,
                       int routing) {
  return ItemBuilder{}
      .withConnectionId(id)
      .withRouting(routing)
      .withName(name)
      .withColour(123456)
      .withContinuityCounter(0)
      .withChanged(true);
}

InputItemMetadata simpleObjectItem(ConnectionId const& id,
                             std::string const& name,
                             int routing) {
  return simpleItem(id, name, routing).withDefaultObjMetadata();
}

InputItemMetadata simpleDsItem(ConnectionId const& id,
                               std::string const& name,
                               int routing,
                               SpeakerLayout layout) {
  return simpleItem(id, name, routing).withDsLayout(layout);
}

}

TEST_CASE("Programme parser tests") {
  Metadata metadata{std::make_unique<EventDispatcher>(), std::make_unique<EventDispatcher>()};
  auto connectionId = ConnectionId::generate();
  auto item = simpleObjectItem(connectionId, "test", 0);
  metadata.setInputItemMetadata(connectionId, item);

  auto programmeStore = StoreBuilder{}.withProgramme(ProgrammeBuilder{}
                                                  .withName("TestProg")
                                                  .withLanguage("English")
                                                  .withObject(connectionId));
  metadata.setStore(programmeStore);

  SECTION("Single Programme, Single object serialized correctly") {
    ProgrammeStoreAdmSerializer serializer;
    auto result = serializer.serialize(metadata.stores());
    auto const& doc = *result.first;
    auto const pluginMap = result.second;
    REQUIRE(numberOf<adm::AudioProgramme>(doc) == 1);
    REQUIRE(numberOf<adm::AudioObject>(doc) == 1);
    REQUIRE(numberOf<adm::AudioTrackUid>(doc) == 1);
    REQUIRE(numberOfUncommon<adm::AudioTrackFormat>(doc) == 1);
    REQUIRE(numberOfUncommon<adm::AudioStreamFormat>(doc) == 1);
    REQUIRE(numberOfUncommon<adm::AudioChannelFormat>(doc) == 1);
    REQUIRE(numberOfUncommon<adm::AudioPackFormat>(doc) == 1);
    /*
    REQUIRE(chna.numUids() == 1);
    REQUIRE(chna.audioIds().size() == 1);
    REQUIRE(chna.numTracks() == 1);
    */
  }

  programmeStore = programmeStore.withProgramme(ProgrammeBuilder{}
                                                    .withName("TestProg2")
                                                    .withLanguage("English")
                                                    .withObject(connectionId));

  metadata.setStore(programmeStore);

  SECTION("Two Programmes, Shared object serialized correctly") {
    ProgrammeStoreAdmSerializer serializer;
    auto result = serializer.serialize(metadata.stores());
    auto const& doc = *result.first;
    auto const pluginMap = result.second;
    REQUIRE(numberOf<adm::AudioProgramme>(doc) == 2);
    REQUIRE(numberOf<adm::AudioObject>(doc) == 1);
    REQUIRE(numberOf<adm::AudioTrackUid>(doc) == 1);
    REQUIRE(numberOfUncommon<adm::AudioTrackFormat>(doc) == 1);
    REQUIRE(numberOfUncommon<adm::AudioStreamFormat>(doc) == 1);
    REQUIRE(numberOfUncommon<adm::AudioChannelFormat>(doc) == 1);
    REQUIRE(numberOfUncommon<adm::AudioPackFormat>(doc) == 1);

    /*
    REQUIRE(chna.numUids() == 1);
    REQUIRE(chna.audioIds().size() == 1);
    REQUIRE(chna.numTracks() == 1);
    */
  }
}

TEST_CASE("Stereo DirectSpeaker input serialized correctly") {

  auto connectionId = ConnectionId::generate();
  auto item = simpleDsItem(connectionId,
                           "testDs",
                           2,
                           SpeakerLayout::ITU_BS_2051_0_2_0);

  Metadata metadata{std::make_unique<EventDispatcher>(), std::make_unique<EventDispatcher>()};
  metadata.setInputItemMetadata(connectionId, item);

  auto programmeStore = StoreBuilder{}.withProgramme(ProgrammeBuilder{}
                                                  .withName("TestProg")
                                                  .withLanguage("English")
                                                  .withObject(connectionId));

  metadata.setStore(programmeStore);

  ProgrammeStoreAdmSerializer serializer;
  auto result = serializer.serialize(metadata.stores());
  auto const& doc = *result.first;
  auto pluginMap = result.second;

  using namespace adm;
  REQUIRE(numberOf<AudioProgramme>(doc) == 1);
  REQUIRE(numberOf<AudioObject>(doc) == 1);
  REQUIRE(numberOf<AudioTrackUid>(doc) == 2); // stereo
  REQUIRE(numberOfUncommon<AudioTrackFormat>(doc) == 0);
  REQUIRE(numberOfUncommon<AudioStreamFormat>(doc) == 0);
  REQUIRE(numberOfUncommon<AudioChannelFormat>(doc) == 0);
  REQUIRE(numberOfUncommon<AudioPackFormat>(doc) == 0);

  /*
  REQUIRE(chna.numUids() == 2);
  REQUIRE(chna.audioIds().size() == 2);
  REQUIRE(chna.numTracks() == 2);
  */
}

namespace {
using namespace adm;
std::shared_ptr<AudioObject const> objectWithName(std::string const& name,
                                            Document const& doc) {
  auto const& objects = doc.getElements<AudioObject>();
  auto it = std::find_if(objects.begin(), objects.end(),
                         [name](std::shared_ptr<AudioObject const> const& obj) {
                           return obj->get<AudioObjectName>().get() == name;
                         });
  if(it != objects.end()) {
    return *it;
  }
  return nullptr;
}

template <typename T>
bool contains(T const& container, typename std::shared_ptr<typename T::value_type::element_type const> const& val) {
  return std::find(container.cbegin(), container.cend(), val) != container.end();
}

}

TEST_CASE("Toggle group with three members") {
  auto defaultId = ConnectionId::generate();
  auto defaultItem = simpleObjectItem(defaultId, "default", 0);
  auto altId = ConnectionId::generate();
  auto altItem = simpleObjectItem(altId, "alternative", 1);
  auto secondAltId = ConnectionId::generate();
  auto secondAltItem = simpleObjectItem(secondAltId, "alternative_2", 2);
  Metadata metadata{std::make_unique<EventDispatcher>(), std::make_unique<EventDispatcher>()};
  metadata.setInputItemMetadata(defaultId, defaultItem);
  metadata.setInputItemMetadata(altId, altItem);
  metadata.setInputItemMetadata(secondAltId, secondAltItem);

  auto toggle = ToggleBuilder{}
      .withDefaultItem(defaultId)
      .withItem(altId)
      .withItem(secondAltId);

  auto programmeStore = StoreBuilder{}.withProgramme(ProgrammeBuilder{}
                                                         .withName("TestProg")
                                                         .withLanguage("English")
                                                         .withToggle(toggle));

  metadata.setStore(programmeStore);

  ProgrammeStoreAdmSerializer serializer;
  auto result = serializer.serialize(metadata.stores());
  auto const& doc = *result.first;
  auto pluginMap = result.second;

  using namespace adm;
  SECTION("creates correct adm elements") {
    REQUIRE(numberOf<AudioProgramme>(doc) == 1);
    REQUIRE(numberOf<AudioObject>(doc) == 3);
    REQUIRE(numberOf<AudioTrackUid>(doc) == 3);  // stereo
    REQUIRE(numberOfUncommon<AudioTrackFormat>(doc) == 3);
    REQUIRE(numberOfUncommon<AudioStreamFormat>(doc) == 3);
    REQUIRE(numberOfUncommon<AudioChannelFormat>(doc) == 3);
    REQUIRE(numberOfUncommon<AudioPackFormat>(doc) == 3);

    /*
    REQUIRE(chna.numUids() == 3);
    REQUIRE(chna.audioIds().size() == 3);
    REQUIRE(chna.numTracks() == 3);
    */
  }

  SECTION("adm structure") {
    auto defaultObject = objectWithName("default", doc);
    auto altObject = objectWithName("alternative", doc);
    auto secondAltObject = objectWithName("alternative_2", doc);
    auto complimentary = defaultObject->getComplementaryObjects();
    SECTION("default toggle object has complimentary references to other toggle objects") {
      REQUIRE(complimentary.size() == 2);
      REQUIRE(contains(complimentary, altObject));
      REQUIRE(contains(complimentary, secondAltObject));
      REQUIRE(!contains(complimentary, defaultObject));
    }
    SECTION("each toggle object in unique content") {
      auto const& contents = doc.getElements<AudioContent>();
      int defaultCount{0}, altCount{0}, secondAltCount{0};
      for(auto content : contents) {
        auto objReferences = content->getReferences<AudioObject>();
        REQUIRE(objReferences.size() <= 1);
        if(!objReferences.empty()) {
          auto const& firstRef = objReferences.front();
          if (firstRef == defaultObject) defaultCount += 1;
          if (firstRef == altObject) altCount += 1;
          if (firstRef == secondAltObject) secondAltCount += 1;
        }
      }
      REQUIRE(defaultCount == 1);
      REQUIRE(altCount == 1);
      REQUIRE(secondAltCount == 1);

      SECTION("and toggle contents named after object") {
        for(auto const& content : contents) {
          auto const& objectReferences = content->getReferences<AudioObject>();
          if(objectReferences.size() == 1) {
            auto contentName = content->get<AudioContentName>().get();
            auto objectName = objectReferences.front()->get<AudioObjectName>().get();
            REQUIRE(contentName == objectName);
          }
        }
      }
    }
  }
}

TEST_CASE("On_Off interactive") {
  Metadata metadata{std::make_unique<EventDispatcher>(), std::make_unique<EventDispatcher>()};
  auto connectionId = ConnectionId::generate();
  auto item = simpleObjectItem(connectionId, "test", 0);
  metadata.setInputItemMetadata(connectionId, item);
  auto programme = ProgrammeBuilder{}
      .withName("TestProg")
      .withLanguage("English")
      .with(ObjectBuilder{}
                .withItem(connectionId)
                .withOnOffInteractionEnabled());

  auto programmeStore = StoreBuilder{}.withProgramme(programme);
  metadata.setStore(programmeStore);

  SECTION("With single object") {
    ProgrammeStoreAdmSerializer serializer;
    auto result = serializer.serialize(metadata.stores());
    auto const& doc = *result.first;
    auto chna = result.second;
    REQUIRE(doc.getElements<AudioObject>().size() == 1);
    auto const& obj = *doc.getElements<AudioObject>().front();
    REQUIRE((obj.has<Interact>() && obj.get<Interact>().get()));
    REQUIRE(obj.has<AudioObjectInteraction>());
    auto const interact = obj.get<AudioObjectInteraction>();
    REQUIRE(interact.has<OnOffInteract>());
    auto const onOff = interact.get<OnOffInteract>();
    REQUIRE(onOff.get());
  }

  SECTION("When same input has same interaction settings in two programmes") {
    programmeStore =
        programmeStore.withProgramme(programme);
    metadata.setStore(programmeStore);
    ProgrammeStoreAdmSerializer serializer;
    auto result = serializer.serialize(metadata.stores());
    auto const& doc = *result.first;
    auto chna = result.second;
    auto const& objects = doc.getElements<AudioObject>();
    REQUIRE(objects.size() == 1);
    auto interactiveObject = objects.front();
    REQUIRE(interactiveObject->get<Interact>().get());
    auto objectInteraction = interactiveObject->get<AudioObjectInteraction>();
    REQUIRE(objectInteraction.get<OnOffInteract>().get());
    for(auto& programme : doc.getElements<AudioProgramme>()) {
      auto contents = programme->getReferences<AudioContent>();
      REQUIRE(contents.size() == 1);
      auto objects = contents.front()->getReferences<AudioObject>();
      REQUIRE(objects.size() == 1);
    }
  }

  SECTION("When same input has different interaction settings in two programmes") {
    programmeStore =
        programmeStore.withProgramme(ProgrammeBuilder{}
                                         .withName("Second Programme")
                                         .withLanguage("English")
                                         .withObject(connectionId));
    metadata.setStore(programmeStore);
    ProgrammeStoreAdmSerializer serializer;
    auto result = serializer.serialize(metadata.stores());
    auto const& doc = *result.first;
    auto chna = result.second;
    auto const& objects = doc.getElements<AudioObject>();
    REQUIRE(objects.size() == 2);
    int interactiveCount = 0;
    std::shared_ptr<AudioObject const> interactiveObject;
    std::shared_ptr<AudioObject const> otherObject;
    for (auto const& obj : objects) {
      if (obj->has<AudioObjectInteraction>()) {
        interactiveObject = obj;
        ++interactiveCount;
      } else {
        otherObject = obj;
      }
    }
    REQUIRE(interactiveCount == 1);
    auto objectInteraction = interactiveObject->get<AudioObjectInteraction>();
    REQUIRE(objectInteraction.get<OnOffInteract>().get());
    auto referencesUid = [](std::shared_ptr<AudioTrackUid const> uid, std::shared_ptr<AudioObject const> obj) {
      auto const& uidRefs = obj->getReferences<AudioTrackUid>();
      return std::find(uidRefs.begin(), uidRefs.end(), uid) != uidRefs.end();
    };
    auto referencesInteractiveUid = std::bind(referencesUid, interactiveObject->getReferences<AudioTrackUid>().front(), _1);
    REQUIRE(referencesInteractiveUid(otherObject));
  }
}

