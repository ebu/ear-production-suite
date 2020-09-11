#pragma once
#include <adm/adm.hpp>
#include <adm/route_tracer.hpp>
#include "programme_store.pb.h"
#include <boost/variant/variant.hpp>
#include <map>
namespace ear {
namespace plugin {
struct StopAtChannel {
  template <typename SubElement, typename Element>
  bool shouldRecurse(std::shared_ptr<Element>,
                     std::shared_ptr<SubElement>) {
    return false;
  }
  bool shouldRecurse(std::shared_ptr<const adm::AudioProgramme>,
                     std::shared_ptr<const adm::AudioContent>) {
    return true;
  }
  bool shouldRecurse(std::shared_ptr<const adm::AudioContent>,
                     std::shared_ptr<const adm::AudioObject>) {
    return true;
  }
  bool shouldRecurse(std::shared_ptr<const adm::AudioObject>,
                     std::shared_ptr<const adm::AudioObject>) {
    return true;
  }
  bool shouldRecurse(std::shared_ptr<const adm::AudioObject>,
                     std::shared_ptr<const adm::AudioTrackUid>) {
    return true;
  }
  bool shouldRecurse(std::shared_ptr<const adm::AudioTrackUid>,
                     std::shared_ptr<const adm::AudioTrackFormat>) {
    return true;
  }
  bool shouldRecurse(std::shared_ptr<const adm::AudioTrackFormat>,
                     std::shared_ptr<const adm::AudioStreamFormat>) {
    return true;
  }
  bool shouldRecurse(std::shared_ptr<const adm::AudioStreamFormat>,
                     std::shared_ptr<const adm::AudioChannelFormat>) {
    return true;
  }

  template <typename Element>
  bool shouldAdd(std::shared_ptr<Element>) {
    return false;
  }
  bool shouldAdd(std::shared_ptr<adm::AudioProgramme const>) {
    return true;
  }
  bool shouldAdd(std::shared_ptr<adm::AudioContent const>) {
    return true;
  }
  bool shouldAdd(std::shared_ptr<adm::AudioObject const>) {
    return true;
  }
  bool shouldAdd(std::shared_ptr<adm::AudioTrackUid const>) {
    return true;
  }
  bool shouldAdd(std::shared_ptr<adm::AudioChannelFormat const>) {
    return true;
  }

  template <typename Element>
  bool isEndOfRoute(std::shared_ptr<Element>) {
    return false;
  }
  bool isEndOfRoute(std::shared_ptr<const adm::AudioChannelFormat>) {
    return true;
  }
};

  class ProgrammeStoreAdmPopulator : public boost::static_visitor<void> {
   public:
    ProgrammeStoreAdmPopulator(proto::ProgrammeStore* store,
                               std::vector<uint32_t> trackMaps)
        : store{store},
          trackMaps{std::move(trackMaps)}
          {};
    void operator()(std::shared_ptr<adm::AudioProgramme const>);
    void operator()(std::shared_ptr<adm::AudioContent const>){/*TODO, groups*/};
    void operator()(std::shared_ptr<adm::AudioObject const>);
    void operator()(std::shared_ptr<adm::AudioTrackUid const>);
    void operator()(std::shared_ptr<adm::AudioPackFormat const>){}
    void operator()(std::shared_ptr<adm::AudioTrackFormat const>){}
    void operator()(std::shared_ptr<adm::AudioStreamFormat const>){}
    void operator()(std::shared_ptr<adm::AudioChannelFormat const>);
    void startRoute();
    void endRoute();

    std::multimap<int, proto::ProgrammeElement*> getTrackLookup() { return elementTrackLookup; }
   private:
    proto::ProgrammeStore* store;
    std::vector<uint32_t> trackMaps;
    std::map<int, proto::ProgrammeElement*> programmeElementTrackLookup;
    std::multimap<int, proto::ProgrammeElement*> elementTrackLookup;
    std::map<std::shared_ptr<adm::AudioProgramme const>, proto::Programme*> importedProgrammes;
    std::shared_ptr<adm::AudioTrackUid const> currentUid;
    std::shared_ptr<adm::AudioObject const> currentObject;
    proto::Programme* currentProgramme;
  };

  std::multimap<int, proto::ProgrammeElement *> populateStoreFromAdm(
      adm::Document const& doc,
      proto::ProgrammeStore& store,
      std::vector<uint32_t> const& trackMaps);
}
}
