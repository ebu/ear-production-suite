#include "admimporter.h"
#include "reaperapi.h"
#include "channelindexer.h"
#include "pcmgroupregistry.h"
#include "pcmsourcecreator.h"
#include "projectelements.h"
#include "projecttree.h"
#include "admmetadata.h"
#include <boost/variant/static_visitor.hpp>
#include <adm/document.hpp>
#include <adm/elements.hpp>
#include <adm/route_tracer.hpp>
#include <bw64/bw64.hpp>
#include "pcmgroup.h"
#include "pcmwriterfactory.h"
#include "pcmreader.h"
#include "pluginsuite.h"
#include "nodefactory.h"
#include "importelement.h"
#include "importaction.h"
#include "progress/importlistener.h"
#include <WDL/swell/swell.h>
#include <exception>

using namespace admplug;

namespace {
  struct FullDepthViaUIDStrategy {

      template <typename SubElement, typename Element>
      bool shouldRecurse(std::shared_ptr<Element> a,
                         std::shared_ptr<SubElement> b) {
        return false;
      }

      bool shouldRecurse(std::shared_ptr<const adm::AudioProgramme> a,
                         std::shared_ptr<const adm::AudioContent> b) {
        return true;
      }
      bool shouldRecurse(std::shared_ptr<const adm::AudioContent> a,
                         std::shared_ptr<const adm::AudioObject> b) {
        return true;
      }

      bool shouldRecurse(std::shared_ptr<const adm::AudioObject> a,
                         std::shared_ptr<const adm::AudioObject> b) {
          return true;
      }

      bool shouldRecurse(std::shared_ptr<const adm::AudioObject> a,
                         std::shared_ptr<const adm::AudioTrackUid> b) {
        return true;
      }
      bool shouldRecurse(std::shared_ptr<const adm::AudioTrackUid> a,
                         std::shared_ptr<const adm::AudioTrackFormat> b) {
          return true;
      }
      bool shouldRecurse(std::shared_ptr<const adm::AudioTrackFormat> a,
                         std::shared_ptr<const adm::AudioStreamFormat> b) {
          return true;
      }
      bool shouldRecurse(std::shared_ptr<const adm::AudioStreamFormat> a,
                         std::shared_ptr<const adm::AudioChannelFormat> b) {
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
      bool isEndOfRoute(std::shared_ptr<Element> e) {
        return false;
      }

      bool isEndOfRoute(std::shared_ptr<const adm::AudioChannelFormat>) {
        return true;
      }

    };

  template <typename ExistingT, typename NewT>
  std::vector<std::shared_ptr<NewT const>> getElementsIfNo(std::shared_ptr<adm::Document const> doc) {
      auto possibleParents = doc->getElements<ExistingT>();
      auto possibleElements = doc->getElements<NewT>();
      std::vector<std::shared_ptr<NewT const>> orphanedElements(possibleElements.size());

      auto it = std::copy_if(possibleElements.begin(), possibleElements.end(), orphanedElements.begin(),
                  [&possibleParents](auto e) {
                      for(auto p : possibleParents) {
                          auto children = p->template getReferences<NewT>();
                          for(auto c : children) {
                              if(c == e) return false;
                          }
                      }
                      return true;
                  });
      orphanedElements.resize(std::distance(orphanedElements.begin(), it));

      return orphanedElements;
  }

  void applyRoute(adm::Route const& route, ProjectTree& project) {
      project.resetRoot();
      for(auto element : route) {
          boost::apply_visitor(project, element);
      }
  }

  template<typename ElementT, typename TracerT>
  void applyRoutes(ElementT elements, TracerT tracer, ProjectTree& project) {
      for(auto element : elements) {
          auto routes = tracer.run(element);
          for(auto& route : routes) {
              applyRoute(route, project);
          }
      }
  }
  constexpr int asInt(ReaperAPI::UndoFlag flag) {
      return static_cast<int>(flag);
  }

  void checkMetadataPresent(IADMMetaData const& admData)
  {
      if (!admData.chna()) {
          throw std::runtime_error("Missing or invalid chna chunk in source file");
      }

      if (!admData.axml()) {
          throw std::runtime_error("Missing or invalid axml chunk in source file");
      }
  }
}

ADMImporter::~ADMImporter() = default;


ADMImporter::ADMImporter(MediaItem* fromMediaItem,
                         std::string fileName,
                         ImportContext context,
                         std::string importPath) :
    originalMediaItem{ fromMediaItem },
    importPath{importPath},
    context{std::move(context)},
    fileName{fileName}
{
}

void ADMImporter::parse() {
    auto& broadcast = *context.broadcast;
    broadcast.setStatus(ImportStatus::PARSING_METADATA);
    try {
        metadata = std::make_shared<ADMMetaData>(fileName);
        checkMetadataPresent(*metadata);
        auto admDoc = metadata->adm();
        programmes = std::vector<std::shared_ptr<adm::AudioProgramme const>>(admDoc->getElements<adm::AudioProgramme>().begin(), admDoc->getElements<adm::AudioProgramme>().end());
        contents = getElementsIfNo<adm::AudioProgramme, adm::AudioContent>(admDoc);
        objects = getElementsIfNo<adm::AudioContent, adm::AudioObject>(admDoc);
        uids = getElementsIfNo<adm::AudioObject, adm::AudioTrackUid>(admDoc);
        auto tracer = adm::detail::GenericRouteTracer<adm::Route, FullDepthViaUIDStrategy>();
        sourceCreator = std::make_shared<PCMSourceCreator>(std::make_unique<PCMGroupRegistry>(),
                                                           std::make_unique<Bw64PCMReader>(fileName),
                                                           std::make_unique<RoutingWriterFactory>(),
                                                           *metadata);
        project = std::make_unique<ProjectTree>(std::make_unique<NodeCreator>(sourceCreator, originalMediaItem),
                                                std::make_unique<ProjectNode>(std::make_unique<ImportElement>(originalMediaItem)),
                                                this->context.broadcast);
        applyRoutes(programmes, tracer, *project);
        applyRoutes(contents, tracer, *project);
        applyRoutes(objects, tracer, *project);
    } catch (std::runtime_error const& e) {
        broadcast.error(e);
    }
    auto& import = *context.import;
    auto status = import.status();
    if(status == ImportStatus::CANCELLED) {
        broadcast.setStatus(ImportStatus::COMPLETE);
    } else if (status != ImportStatus::ERROR_OCCURRED) {
        extractAudio();
    }
}

void ADMImporter::extractAudio() {
    auto& import = *context.import;
    auto& broadcast = *context.broadcast;
    try {
      broadcast.setStatus(ImportStatus::EXTRACTING_AUDIO);
      sourceCreator->extractSources(importPath, context);
      auto status = import.status();
      if(status == ImportStatus::CANCELLED) {
          broadcast.setStatus(ImportStatus::COMPLETE);
      } else {
          broadcast.setStatus(ImportStatus::AUDIO_READY);
      }
    } catch (std::runtime_error const& e) {
        broadcast.error(e);
    }
}

void ADMImporter::buildProject() {
    auto const& api = context.api;
    auto currentProject = api.getCurrentProject();
    api.Undo_BeginBlock2(currentProject);

    constexpr int undoFlags = asInt(ReaperAPI::UndoFlag::FX) +
                              asInt(ReaperAPI::UndoFlag::MISC_CONFIG) +
                              asInt(ReaperAPI::UndoFlag::TRACK_ITEMS) +
                              asInt(ReaperAPI::UndoFlag::TRACK_ENVELOPES) +
                              asInt(ReaperAPI::UndoFlag::TRACK_CONFIG) +
                              asInt(ReaperAPI::UndoFlag::POOLED_ENVELOPES);

    try {
        context.pluginSuite->onProjectBuildBegin(metadata, api);
        sourceCreator->linkSources(api);
        project->create(*context.pluginSuite, api);
        if (originalMediaItem != nullptr) {
            auto track = api.GetMediaItem_Track(originalMediaItem);
            if (track != nullptr) {
                api.disableTrackMasterSend(track);
            }
        }
        context.pluginSuite->onProjectBuildComplete(api);
        api.Undo_EndBlock2(currentProject, "ADM Import", undoFlags);
        context.broadcast->setStatus(ImportStatus::COMPLETE);
    } catch (std::runtime_error const& error) {
        api.Undo_EndBlock2(currentProject, "ADM Import", undoFlags);
        context.broadcast->error(error);
        context.broadcast->setStatus(ImportStatus::ERROR_OCCURRED);

    }

}
