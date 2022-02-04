#pragma once

#include <string>
#include <memory>
#include "pluginsuite.h"
#include "projectelements.h"

namespace admplug {
class Track;
class PluginParameter;

class EARPluginSuite : public PluginSuite
{

public:
	EARPluginSuite();
	~EARPluginSuite();

	void onProjectBuildBegin(std::shared_ptr<IADMMetaData> metadata, const ReaperAPI& api) override;
	void onCreateProject(const ProjectNode& rootNode, const ReaperAPI& api) override;
	void onCreateObjectTrack(const TrackElement&, const ReaperAPI& api) override;
	void onCreateDirectTrack(const TrackElement&, const ReaperAPI& api) override;
	void onCreateHoaTrack(const TrackElement&, const ReaperAPI& api) override;
	void onCreateGroup(const TrackElement&, const ReaperAPI& api) override;
	void onObjectAutomation(const ObjectAutomation&, const ReaperAPI& api) override;
	void onDirectSpeakersAutomation(const DirectSpeakersAutomation&, const ReaperAPI& api) override;
	void onHoaAutomation(const HoaAutomation&, const ReaperAPI& api) override;
	void onProjectBuildComplete(const ReaperAPI& api) override;
	bool pluginSuiteUsable(const ReaperAPI& api) override;
	bool representAdmStructureWithGroups(ReaperAPI const& api) override;
	bool applyFXPreset(const HoaAutomation&, const ReaperAPI& api) override;
	PluginParameter* getPluginParameterFor(AdmParameter admParameter) override;
	Parameter* getParameterFor(AdmParameter admParameter) override;
	std::vector<ADMChannel> reorderAndFilter(std::vector<ADMChannel> const& channels, ReaperAPI const& api) override;

	static const char* OBJECT_METADATA_PLUGIN_NAME;
	static const char* DIRECTSPEAKERS_METADATA_PLUGIN_NAME;
	static const char* HOA_METADATA_PLUGIN_NAME;
	static const char* SCENEMASTER_PLUGIN_NAME;
	static const char* RENDERER_PLUGIN_NAME;

private:
	std::vector<std::unique_ptr<PluginParameter>> const& automatedObjectPluginParameters();
	std::vector<std::unique_ptr<TrackParameter>> const& trackParameters();
	std::shared_ptr<PluginParameter> objectTrackMappingParameter;
	std::shared_ptr<PluginParameter> directSpeakersTrackMappingParameter;
    std::shared_ptr<PluginParameter> directPackFormatIdValueParameter;
	std::shared_ptr<PluginParameter> hoaTrackMappingParameter;
	std::shared_ptr<PluginParameter> hoaPackFormatIdValueParameter;
	std::unique_ptr<UniqueValueAssigner> trackMappingAssigner;

	std::unique_ptr<Track> createBusTrack(std::string pluginName, const ReaperAPI& api);
	std::unique_ptr<Track> sceneMasterTrack;
	std::unique_ptr<Track> rendererTrack;
	void checkForExistingTracks(const ReaperAPI& api);
	int trackInsertionIndex{ -1 };
	void setTrackInsertionIndexFromSelectedMedia(const ReaperAPI& api);

	std::string originalAdmDocument;
	std::vector<uint32_t> trackMappingToAtu;
	bool sceneMasterAlreadyExisted{ false };

	static const int MAX_CHANNEL_COUNT;

	static bool registered;
};
}
