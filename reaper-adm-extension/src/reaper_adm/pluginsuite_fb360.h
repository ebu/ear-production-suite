#pragma once

#include <vector>
#include <adm/adm.hpp>
#include <memory>
#include "pluginsuite.h"
#include "projectelements.h"

namespace admplug {
    class PluginParameter;
    class TrackParameter;
    class Plugin;
    class PluginInstance;
    class Track;
    class CommonTrackPool;

    class Facebook360PluginSuite : public PluginSuite {

    public:
        Facebook360PluginSuite();
        ~Facebook360PluginSuite();

        void onCreateProject(const ProjectNode &rootNode, const ReaperAPI &api) override;
        void onCreateObjectTrack(TrackElement &trackNode, const ReaperAPI &api) override;
        void onCreateDirectTrack(TrackElement &trackNode, const ReaperAPI &api) override;
        void onCreateGroup(TrackElement &trackNode, const ReaperAPI &api) override;
        void onObjectAutomation(ObjectAutomation const&, ReaperAPI const& api) override;
        void onDirectSpeakersAutomation(DirectSpeakersAutomation const&, ReaperAPI const& api) override;
        void onHoaAutomation(HoaAutomation const&, ReaperAPI const& api) override;
        void onCreateHoaTrack(TrackElement &trackNode, const ReaperAPI &api) override;
        bool pluginSuiteUsable(ReaperAPI const& api) override;
        bool applyFXPreset(const HoaAutomation &, const ReaperAPI &api) override;
        PluginParameter* getPluginParameterFor(AdmParameter admParameter) override;
        TrackParameter* getTrackParameterFor(AdmParameter admParameter) override;
        Parameter* getParameterFor(AdmParameter admParameter) override;
        const RequiresAdmExportVst pluginSuiteRequiresAdmExportVst() override { return RequiresAdmExportVst::PRE_SPATIALISATION; } // TODO - this might be type specific - e.g, HOA can be pre/post.
        bool pluginUsesSphericalCoordinates(PluginInstance* pluginInst) override { return true; }
        std::optional<std::string> getSpatialisationPluginNameForObjects() override;
        std::optional<std::string> getSpatialisationPluginNameForDirectSpeakers() override;
        std::optional<std::string> getSpatialisationPluginNameForHoa() override;
    private:
        std::vector<std::unique_ptr<PluginParameter>> const & automatedObjectPluginParameters() const;
        std::vector<std::unique_ptr<TrackParameter>> const & trackParameters() const;
        void doGenericTrackSetup(Track& track);
        void setupMetadataPlugin(Track& track, ReaperAPI const& api);
        void applyParameters(const ObjectAutomation &element,
                             const Track &track,
                             const Plugin &plugin) const;

        Track& getCommonDefinitionTrack(const DirectSpeakersAutomation &element, const ReaperAPI &api);
        std::unique_ptr<Track> getControlTrack(const ReaperAPI &api);
        void setAsGroupsSlave(const TrackElement &element, Track &track);

        std::unique_ptr<Track> busTrack3D;
        std::unique_ptr<Track> controlTrack;
        int trackInsertionIndex{ -1 };
        std::unique_ptr<CommonTrackPool> commonTracks;

        static const char* OBJECT_METADATA_PLUGIN_NAME;
        static const char* RENDERER_PLUGIN_NAME;

        static bool registered;
    };

}

