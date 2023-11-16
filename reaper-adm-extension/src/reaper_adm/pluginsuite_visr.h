#pragma once

#include <vector>
#include <adm/adm.hpp>
#include <memory>
#include "pluginsuite.h"
#include "projectelements.h"

namespace admplug {
    class PluginParameter;
    class TrackParameter;
    class Track;
    class CommonTrackPool;

    class VisrPluginSuite : public PluginSuite {

    public:
        VisrPluginSuite();
        ~VisrPluginSuite();

        void onCreateProject(const ProjectNode &rootNode, const ReaperAPI &api) override;
        void onCreateObjectTrack(TrackElement &trackNode, const ReaperAPI &api) override;
        void onCreateDirectTrack(TrackElement &trackNode, const ReaperAPI &api) override;
        void onCreateHoaTrack(TrackElement &trackNode, const ReaperAPI &api) override;
        void onCreateGroup(TrackElement &trackNode, const ReaperAPI &api) override;
        void onObjectAutomation(ObjectAutomation const&, ReaperAPI const& api) override;
        void onDirectSpeakersAutomation(DirectSpeakersAutomation const&, ReaperAPI const& api) override;
        void onHoaAutomation(HoaAutomation const&, ReaperAPI const& api) override;
        bool pluginSuiteUsable(ReaperAPI const& api) override;
        bool applyFXPreset(const HoaAutomation &, const ReaperAPI &api) override;
        PluginParameter* getPluginParameterFor(AdmParameter admParameter) override;
        TrackParameter* getTrackParameterFor(AdmParameter admParameter) override;
        Parameter* getParameterFor(AdmParameter admParameter) override;
        const RequiresAdmExportVst pluginSuiteRequiresAdmExportVst() override { return RequiresAdmExportVst::PRE_SPATIALISATION; }
        bool pluginUsesSphericalCoordinates(PluginInstance* pluginInst) override { return true; }
        std::optional<std::string> getSpatialisationPluginNameForObjects() override;
        std::optional<std::string> getSpatialisationPluginNameForDirectSpeakers() override;

    private:
        std::vector<std::unique_ptr<PluginParameter>> const & automatedObjectPluginParameters();
        std::vector<std::unique_ptr<TrackParameter>> const & trackParameters();
        std::shared_ptr<PluginParameter> objectIdParameter;
        std::unique_ptr<UniqueValueAssigner> objectIdAssigner;

        void doGenericTrackSetup(Track& track);
        void setupTrackWithMetadataPlugin(Track &track, ReaperAPI const& api);
        Track &getCommonTrack(const DirectSpeakersAutomation &element, const ReaperAPI &api);

        std::unique_ptr<Track> busTrack3D;
        std::unique_ptr<CommonTrackPool> commonTracks;
        int trackInsertionIndex{ -1 };

        static const char* OBJECT_METADATA_PLUGIN_NAME;
        static const char* SCENEMASTER_PLUGIN_NAME;
        static const char* RENDERER_PLUGIN_NAME;
        static const int MAX_CHANNEL_COUNT;

        static bool registered;
    };

}
