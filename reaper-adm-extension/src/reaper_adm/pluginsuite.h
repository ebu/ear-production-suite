#pragma once
#include <optional>
#include <vector>
#include "reaperapi.h"
#include "parameter.h"
#include "plugin.h"
#include "admmetadata.h"
#include "admchannel.h"
#include <adm/adm.hpp>
#include "helper/common_definition_helper.h"

namespace admplug {
    class ProjectNode;
    class ObjectAutomation;
    class DirectSpeakersAutomation;
    class HoaAutomation;
    class TrackElement;
    class TakeElement;
    class ReaperAPI;

    class PluginSuite {
    public:
        virtual ~PluginSuite() {}//TODO = default;
        enum RequiresAdmExportVst {
            NONE = 0,
            PRE_SPATIALISATION,
            POST_SPATIALISATION
        };

        virtual void onProjectBuildBegin(std::shared_ptr<IADMMetaData> metadata, const ReaperAPI &api) {}
        virtual void onCreateProject(const ProjectNode &rootNode, const ReaperAPI &api) {}//TODO = 0;
        virtual void onCreateObjectTrack(TrackElement const&, ReaperAPI const& api) {}//TODO = 0;
        virtual void onCreateDirectTrack(TrackElement const&, ReaperAPI const& api) {}//TODO = 0;
        virtual void onCreateGroup(TrackElement const&, ReaperAPI const& api) {}//TODO = 0;
        virtual void onObjectAutomation(ObjectAutomation const&, ReaperAPI const& api) {}//TODO = 0;
        virtual void onDirectSpeakersAutomation(DirectSpeakersAutomation const&, ReaperAPI const& api) {}//TODO = 0;
        virtual void onHoaAutomation(HoaAutomation const&, ReaperAPI const& api) {}//TODO = 0;
        virtual void onCreateHoaTrack(TrackElement const&, ReaperAPI const& api) {}//TODO = 0;
        virtual void onProjectBuildComplete(const ReaperAPI &api) {}
        virtual bool pluginSuiteUsable(ReaperAPI const& api) { return false;  } // Use this method to return true or false as to whether the pluginsuite can be used (i.e, all required VSTs are present)
        virtual bool representAdmStructureWithGroups(ReaperAPI const& api) { return true;  } // Use this method to return true or false as to whether the pluginsuite requires groups to represent higher-level ADM elements
        virtual std::vector<ADMChannel> reorderAndFilter(std::vector<ADMChannel> const &channels, ReaperAPI const& api);
        virtual bool applyFXPreset(HoaAutomation const&, ReaperAPI const& api) = 0;
        virtual PluginParameter* getPluginParameterFor(AdmParameter admParameter) { return nullptr; }
        virtual TrackParameter* getTrackParameterFor(AdmParameter admParameter) { return nullptr; }
        virtual Parameter* getParameterFor(AdmParameter admParameter) { return nullptr; }
        virtual const RequiresAdmExportVst pluginSuiteRequiresAdmExportVst() { return RequiresAdmExportVst::NONE; }
        virtual bool pluginUsesSphericalCoordinates(PluginInstance* pluginInst) { return true; } // false = cartesian. Used by export to determine which coordinates to use if both are supported. A plugin instance can be passed in, since it may depend upon the state of a parameter.
        virtual std::optional<std::string> getSpatialisationPluginNameForObjects() { return std::optional<std::string>(); }
        virtual std::optional<std::string> getSpatialisationPluginNameForBinaural() { return std::optional<std::string>(); }
        virtual std::optional<std::string> getSpatialisationPluginNameForDirectSpeakers() { return std::optional<std::string>(); }
        virtual std::optional<std::string> getSpatialisationPluginNameForMatrix() { return std::optional<std::string>(); }
        virtual std::optional<std::string> getSpatialisationPluginNameForHoa() { return std::optional<std::string>(); }

        std::optional<std::string> getSpatialisationPluginNameFor(adm::TypeDescriptor typeDescriptor);

    protected:
        int getHoaOrder(std::vector<admplug::ADMChannel> const& channels);

    };

    // PluginSuite Helpers

    class UniqueValueAssigner {
        // This class handles cases where each plugin should be assigned a unique value for a certain parameter
        // (see objectId for VISR, or trackMapping for EAR)

    public:
        struct SearchCandidate {
            std::string pluginName;
            std::function<std::vector<int>(PluginInstance&)> determineUsedValues; // For a given PluginInstance, this function should return the used values.
        };

        UniqueValueAssigner(UniqueValueAssigner::SearchCandidate searchCriteria, int minVal, int maxVal, const ReaperAPI &api);
        UniqueValueAssigner(std::vector<UniqueValueAssigner::SearchCandidate> searchCriteria, int minVal, int maxVal, const ReaperAPI &api);
        ~UniqueValueAssigner();

        void updateAvailableValues(const ReaperAPI &api);
        std::optional<int> getNextAvailableValue(int numberOfConsectivesRequired = 1);

    private:
        std::vector<SearchCandidate> searchCriteria;
        int minPossible;
        int maxPossible;
        int offset;

        std::vector<int> availableValues;

        std::optional<std::size_t> indexFromVal(int realValue);

    };

}
