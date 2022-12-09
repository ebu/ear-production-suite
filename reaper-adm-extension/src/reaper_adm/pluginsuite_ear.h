#pragma once

#include <string>
#include <memory>
#include <map>
#include <optional>
#include <mutex>
#include <functional>
#include "pluginsuite.h"
#include "projectelements.h"
#include "helper/nng_wrappers.h"

namespace admplug {
class Track;
class PluginParameter;

class EARPluginInstanceIdProvider
{
public:
    EARPluginInstanceIdProvider();
    ~EARPluginInstanceIdProvider();
    static std::shared_ptr<EARPluginInstanceIdProvider> getInstance();

    // Called by EARPluginSuite
    void expectRequest();
    bool waitForRequest(uint16_t maxMs);
    std::optional<uint32_t> getLastProvidedId();
    uint32_t getNextAvailableId();

    // Called via API
    uint32_t provideId();

private:

    std::mutex lastProvidedIdMutex;
    std::optional<uint32_t> lastProvidedId;
    bool awaitingRequest{ false };

};

class EARPluginCallbackHandler
{
public:
    EARPluginCallbackHandler();
    ~EARPluginCallbackHandler();
    static std::shared_ptr<EARPluginCallbackHandler> getInstance();

    // Called by EARPluginSuite
    void reset();
    bool waitForPluginResponse(uint16_t maxMs);
    bool sendData(std::string const &xmlState);

    // Called via API
    void setPluginCallback(std::function<void(std::string const&)> callback);

private:

    std::mutex activeCallbackMutex;
    std::optional<std::function<void(std::string const&)>> activeCallback;
};

class EARPluginSuite : public PluginSuite
{

public:
	EARPluginSuite();
	~EARPluginSuite();

	void onProjectBuildBegin(std::shared_ptr<IADMMetaData> metadata, const ReaperAPI& api) override;
	void onCreateProject(const ProjectNode& rootNode, const ReaperAPI& api) override;
	void onCreateObjectTrack(TrackElement&, const ReaperAPI& api) override;
	void onCreateDirectTrack(TrackElement&, const ReaperAPI& api) override;
	void onCreateHoaTrack(TrackElement&, const ReaperAPI& api) override;
	void onCreateGroup(TrackElement&, const ReaperAPI& api) override;
	void onProjectBuildComplete(const ReaperAPI& api) override;
	bool pluginSuiteUsable(const ReaperAPI& api) override;
	bool representAdmStructureWithGroups(ReaperAPI const& api) override;
	bool applyFXPreset(const HoaAutomation&, const ReaperAPI& api) override;
	PluginParameter* getPluginParameterFor(AdmParameter admParameter) override;
	Parameter* getParameterFor(AdmParameter admParameter) override;

	static const char* OBJECT_METADATA_PLUGIN_NAME;
	static const char* DIRECTSPEAKERS_METADATA_PLUGIN_NAME;
	static const char* HOA_METADATA_PLUGIN_NAME;
	static const char* SCENEMASTER_PLUGIN_NAME;
	static const char* RENDERER_PLUGIN_NAME;

private:
	std::vector<std::unique_ptr<PluginParameter>> const& automatedObjectPluginParameters();
	std::shared_ptr<PluginParameter> objectTrackMappingParameter;
	std::shared_ptr<PluginParameter> directSpeakersTrackMappingParameter;
	std::shared_ptr<PluginParameter> directPackFormatIdValueParameter;
	std::shared_ptr<PluginParameter> hoaTrackMappingParameter;
	std::shared_ptr<PluginParameter> hoaPackFormatIdValueParameter;
	std::unique_ptr<UniqueValueAssigner> trackMappingAssigner;

    struct TrackInfo {
        std::shared_ptr<TrackInstance> track;
        std::optional<int> routingStartChannel;
    };
    std::map<std::shared_ptr<TakeElement>, TrackInfo> takesOnTracks;

	std::unique_ptr<Track> createBusTrack(std::string pluginName, const ReaperAPI& api);
	std::unique_ptr<Track> sceneMasterTrack;
	std::unique_ptr<Track> rendererTrack;
	void checkForExistingTracks(const ReaperAPI& api);
	int trackInsertionIndex{ -1 };
	void setTrackInsertionIndexFromSelectedMedia(const ReaperAPI& api);
    std::unique_ptr<Plugin> createAndNamePlugin(std::string const& pluginName, TrackInstance* track, TrackElement* trackElement, int32_t routingToScene);

    std::vector<PluginToAdmMap> pluginToAdmMaps;

	std::string originalAdmDocument;
	bool sceneMasterAlreadyExisted{ false };

	static const int MAX_CHANNEL_COUNT;

	static bool registered;
};
}
