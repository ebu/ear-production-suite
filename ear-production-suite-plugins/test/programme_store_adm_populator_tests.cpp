#include <catch2/catch_all.hpp>
#include <programme_store.pb.h>
#include <scene_store.pb.h>
#include <programme_store_adm_populator.hpp>

#include <adm/adm.hpp>
#include <adm/common_definitions.hpp>
#include <adm/utilities/id_assignment.hpp>
#include <adm/utilities/object_creation.hpp>

using namespace ear::plugin;

TEST_CASE("Simple ADM") {
  auto admDocument = adm::Document::create();
  auto admProgramme =
      adm::AudioProgramme::create(adm::AudioProgrammeName("Simple Programme"));
  auto admContent =
      adm::AudioContent::create(adm::AudioContentName("Simple Content"));
  admProgramme->addReference(admContent);
  auto holder = adm::createSimpleObject("Simple Object");
  admContent->addReference(holder.audioObject);
  admDocument->add(admProgramme);
  reassignIds(admDocument);

  proto::ProgrammeStore pendingStore;

  SECTION("with single object") {
    auto pendingElements = populateStoreFromAdm(*admDocument, pendingStore, {1});
    REQUIRE(pendingElements.size() == 1);
    REQUIRE(pendingStore.programme_size() == 1);
  }
  SECTION("with single object with on_off interactivity") {
    auto objectInteraction = adm::AudioObjectInteraction{adm::OnOffInteract{true}};
    holder.audioObject->set(objectInteraction);
    holder.audioObject->set(adm::Interact{true});
    auto pendingElements = populateStoreFromAdm(*admDocument, pendingStore, {1});
    REQUIRE(pendingElements.size() == 1);
    REQUIRE(pendingStore.programme_size() == 1);
    REQUIRE(pendingStore.programme(0).element_size() == 1);
    REQUIRE(pendingStore.programme(0).element(0).has_object());
    auto const& object = pendingStore.programme(0).element(0).object();
    REQUIRE(object.has_interactive_on_off());
    REQUIRE(object.interactive_on_off().enabled());
  }
}

TEST_CASE("Object in multiple programmes") {
  auto admDocument = adm::Document::create();
  auto admProgramme1 =
      adm::AudioProgramme::create(adm::AudioProgrammeName("Programme 1"));
  auto admProgramme2 =
      adm::AudioProgramme::create(adm::AudioProgrammeName("Programme 2"));
  auto admContent1 =
      adm::AudioContent::create(adm::AudioContentName("Content 1"));
  auto admContent2 =
      adm::AudioContent::create(adm::AudioContentName("Content 2"));
  admProgramme1->addReference(admContent1);
  admProgramme2->addReference(admContent1);
  auto holder = adm::createSimpleObject("Simple Object");
  admContent1->addReference(holder.audioObject);
  admContent2->addReference(holder.audioObject);
  admDocument->add(admProgramme1);
  admDocument->add(admProgramme2);
  reassignIds(admDocument);

  proto::ProgrammeStore pendingStore;

  auto pendingElements = populateStoreFromAdm(*admDocument, pendingStore, {1});

  REQUIRE(pendingElements.size() == 2);
  REQUIRE(pendingStore.programme_size() == 2);
  REQUIRE(pendingStore.programme(0).element_size() == 1);
  REQUIRE(pendingStore.programme(1).element_size() == 1);
}

TEST_CASE("simple DirectSpeaker") {
  auto admDocument = adm::Document::create();
  adm::addCommonDefinitionsTo(admDocument);
  auto admProgramme =
      adm::AudioProgramme::create(adm::AudioProgrammeName("Programme"));
  admDocument->add(admProgramme);
  auto admContent =
      adm::AudioContent::create(adm::AudioContentName("Content"));
  admProgramme->addReference(admContent);
  auto holder = adm::addSimpleCommonDefinitionsObjectTo(admDocument,
                                                        "DirectSpeakers",
                                                        "0+5+0");
  admContent->addReference(holder.audioObject);

  proto::ProgrammeStore pendingStore;
  auto pendingElements = populateStoreFromAdm(*admDocument,
                                              pendingStore,
                                              {1, 2, 3, 4, 5});

  REQUIRE(pendingElements.size() == 1);
  REQUIRE(pendingStore.programme_size() == 1);
  REQUIRE(pendingStore.programme(0).element_size() == 1);
}

