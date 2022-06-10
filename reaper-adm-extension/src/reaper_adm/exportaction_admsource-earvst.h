#pragma once

#include "reaperapi.h"
#include "exportaction_admsourcescontainer.h"
#include "communicators.h"
#include "parameter.h"
#include "exportaction_issues.h"
#include "exportaction_parameterprocessing.h"

#include <memory>

#define COMMONDEFINITIONS_MAX_ID 0x0FFF

using namespace admplug;

class EarVstCommunicator : public CommunicatorBase
{
public:
	EarVstCommunicator(int samplesPort, int commandPort);
	~EarVstCommunicator() {}

	void updateInfo() override;

	// Existing is fine - bool nextFrameAvailable();
	bool copyNextFrame(float* buf, bool bypassAvailabilityCheck = false) override;

	struct ChannelMapping {
		uint8_t originalChannelNumber;
		uint8_t writtenChannelNumber;
		uint32_t audioTrackUidValue;
	};

	int getReportedSampleRate() override { return sampleRate; }
	int getReportedChannelCount() override { return channelMappings.size(); }
	std::string getAdmTemplateStr() { return admStr; }
	std::vector<ChannelMapping> getChannelMappings() { return channelMappings; }

	void sendAdmAndMappings(std::string originalAdmStr, std::vector<uint32_t> trackMappingToAtu, std::vector<uint32_t> trackMappingToAo);

private:
	void infoExchange();
	void admAndMappingExchange();

	std::string admStr;
	std::vector<ChannelMapping> channelMappings;

	uint8_t channelCount{ 0 };
	uint32_t sampleRate{ 0 };
};

#define EARSCENEMASTER_VST_COMMANDPORTPARAM 0
#define EARSCENEMASTER_VST_SAMPLESPORTPARAM 1
#define EARSCENEMASTER_VST_PORTPARAM_NORMLIMIT 65535.0

class EarInputVst : public PluginInstance
{
public:
	EarInputVst(MediaTrack* mediaTrack, char* vstName, ReaperAPI const& api);
	EarInputVst(MediaTrack* mediaTrack, int fxIndex, ReaperAPI const& api);
	~EarInputVst() {};

	int getTrackMapping();
	int getWidth();

	// Statics

	static const char* getDirectSpeakersVstCompName();
	static const std::string* getDirectSpeakersVstNameStr();
	static bool isDirectSpeakersVstAvailable(ReaperAPI const& api, bool doRescan = true);
	static int trackDirectSpeakersVstIndex(ReaperAPI const& api, MediaTrack* trk);
	static std::vector<int> trackDirectSpeakersVstIndexes(ReaperAPI const& api, MediaTrack* trk);
	static bool vstPosIsDirectSpeakersVst(ReaperAPI const& api, MediaTrack* trk, int vstPos);

	static const char* getObjectVstCompName();
	static const std::string* getObjectVstNameStr();
	static bool isObjectVstAvailable(ReaperAPI const& api, bool doRescan = true);
	static int trackObjectVstIndex(ReaperAPI const& api, MediaTrack* trk);
	static std::vector<int> trackObjectVstIndexes(ReaperAPI const& api, MediaTrack* trk);
	static bool vstPosIsObjectVst(ReaperAPI const& api, MediaTrack* trk, int vstPos);

	static const char* getHoaVstCompName();
	static const std::string* getHoaVstNameStr();
	static bool isHoaVstAvailable(ReaperAPI const& api, bool doRescan = true);
	static int trackHoaVstIndex(ReaperAPI const& api, MediaTrack* trk);
	static std::vector<int> trackHoaVstIndexes(ReaperAPI const& api, MediaTrack* trk);
	static bool vstPosIsHoaVst(ReaperAPI const& api, MediaTrack* trk, int vstPos);

	static bool isObjectPlugin(std::string vstNameStr);
	static bool isDirectSpeakersPlugin(std::string vstNameStr);
	static bool isHoaPlugin(std::string vstNameStr);
	static bool isInputPlugin(char* vstName);
	static bool isInputPlugin(ReaperAPI const& api, MediaTrack* trk, int vstPos);

private:
	// TODO: Fix hard-coded values - pull from plugin suite
    std::unique_ptr<PluginParameter> paramTrackMapping{ createPluginParameter(0, { -1.0, 63.0 }) };
    std::unique_ptr<PluginParameter> paramDirectSpeakersPackFormatIdValue{ createPluginParameter(1, { 0x0, 0xFFFF }) };
    std::unique_ptr<PluginParameter> paramHoaPackFormatIdValue{ createPluginParameter(1, { 0x0, 0xFFFF }) };

	// Statics

	static std::string directSpeakersVstName; // Human-readable name
	static std::string directSpeakersVstCompName; // Name for comparison purposes using API funcs
	static size_t directSpeakersVstCompNameLen;
	static const char* directSpeakersVstCompNameCStr;

	static std::string objectVstName; // Human-readable name
	static std::string objectVstCompName; // Name for comparison purposes using API funcs
	static size_t objectVstCompNameLen;
	static const char* objectVstCompNameCStr;

	static std::string hoaVstName; // Human-readable name
	static std::string hoaVstCompName; // Name for comparison purposes using API funcs
	static size_t hoaVstCompNameLen;
	static const char* hoaVstCompNameCStr;
};

class EarSceneMasterVst : public PluginInstance
{
public:
	EarSceneMasterVst(MediaTrack* mediaTrack, ReaperAPI const& api);
	EarSceneMasterVst(MediaTrack* mediaTrack, int fxIndex, ReaperAPI const& api);
	~EarSceneMasterVst();

	int getSampleRate();
	int getChannelCount();
	std::string getAdmTemplateStr();
	std::vector<EarVstCommunicator::ChannelMapping> getChannelMappings();
	bool getRenderInProgressState();
	void setRenderInProgressState(bool state);

	EarVstCommunicator* getCommunicator(bool mustExist = false);
	int getSamplesSocketPort();
	int getCommandSocketPort();

	// Statics
	static const char* getVstCompName();
	static const std::string* getVstNameStr();
	static bool isAvailable(ReaperAPI const& api, bool doRescan = true);
	static bool isCandidateForExport(std::shared_ptr<EarSceneMasterVst> possibleCandidate);
	static int trackEarSceneMasterVstIndex(ReaperAPI const& api, MediaTrack* trk);
	static std::vector<int> trackEarSceneMasterVstIndexes(ReaperAPI const& api, MediaTrack* trk);
	static bool vstPosIsEarSceneMasterVst(ReaperAPI const& api, MediaTrack* trk, int vstPos);

private:
	std::shared_ptr<EarVstCommunicator> communicator;
	bool isCommunicatorPresent();
	bool obtainCommunicator();
	void releaseCommunicator();

	std::unique_ptr<PluginParameter> paramSamplesPort{ createPluginParameter(EARSCENEMASTER_VST_SAMPLESPORTPARAM,
																					ParameterRange{ 0.0, EARSCENEMASTER_VST_PORTPARAM_NORMLIMIT }) };
	std::unique_ptr<PluginParameter> paramCommandPort{ createPluginParameter(EARSCENEMASTER_VST_COMMANDPORTPARAM,
																					ParameterRange{ 0.0, EARSCENEMASTER_VST_PORTPARAM_NORMLIMIT }) };

	// Statics
	static std::string vstName; // Human-readable name
	static std::string vstCompName; // Name for comparison purposes using API funcs
	static size_t vstCompNameLen;
	static const char* vstCompNameCStr;
};

class EarVstExportSources : public IExportSources
{
	/*
	A container for handling ADM sources provided by EAR SceneMaster VSTs
	*/
public:
	EarVstExportSources(ReaperAPI const& api);
	~EarVstExportSources() {};

	bool documentRequiresProgrammeTitles() { return false; }
	bool documentRequiresContentTitles() { return false; }

	bool validForExport() { return chosenCandidateForExport != nullptr; }
	std::vector<std::shared_ptr<EarSceneMasterVst>>* getAllFoundVsts() { return &allEarSceneMasterVsts; }

	int getSampleRate();
	int getTotalExportChannels();

	void setRenderInProgress(bool state);
	bool isFrameAvailable();
	bool writeNextFrameTo(float* bufferWritePointer, bool skipFrameAvailableCheck = false);

	std::shared_ptr<bw64::AxmlChunk> getAxmlChunk() { return axmlChunk; }
	std::shared_ptr<bw64::ChnaChunk> getChnaChunk() { return chnaChunk; }
	std::shared_ptr<adm::Document> getAdmDocument() { return admDocument; }

	std::vector<std::string> generateExportInfoStrings() { return infoStrings; }
	std::vector<std::string> generateExportErrorStrings() { return errorStrings; }
	std::vector<std::string> generateExportWarningStrings() { return warningStrings; }

	std::string getExportSourcesName() override {
		return std::string("EAR Scene VST");
	}

private:
	void generateAdmAndChna(ReaperAPI const& api);

	struct AdmElements {
		bool isUsingCommonDefinition;
		adm::TypeDescriptor typeDescriptor;
		std::shared_ptr<adm::AudioTrackUid> audioTrackUid;
		std::shared_ptr<adm::AudioTrackFormat> audioTrackFormat;
		std::shared_ptr<adm::AudioPackFormat> audioPackFormat;
		std::shared_ptr<adm::AudioChannelFormat> audioChannelFormat;
		std::shared_ptr<adm::AudioObject> audioObject;
	};
	std::map<uint32_t, AdmElements> admElementsForAudioTrackUidValue;
	std::optional<AdmElements> getAdmElementsFor(uint32_t audioTrackUidValue);
	void updateAdmElementsForAudioTrackUidValueMap();

	std::shared_ptr<adm::Document> admDocument;
	std::shared_ptr<bw64::ChnaChunk> chnaChunk;
	std::shared_ptr<bw64::AxmlChunk> axmlChunk;
	std::vector<std::shared_ptr<EarSceneMasterVst>> allEarSceneMasterVsts{};
	std::vector<std::shared_ptr<EarSceneMasterVst>> candidatesForExport{};
	std::shared_ptr<EarSceneMasterVst> chosenCandidateForExport{};
	std::vector<std::string> infoStrings;
	std::vector<std::string> errorStrings;
	std::vector<std::string> warningStrings;

	// New methods that might need a seperate handler
	std::vector<std::shared_ptr<PluginInstance>> getEarInputPluginsWithTrackMapping(int trackMapping, ReaperAPI const& api);
	TrackEnvelope* getEnvelopeFor(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance* pluginInst, AdmParameter admParameter, ReaperAPI const& api);
	std::optional<double> getValueFor(std::shared_ptr<admplug::PluginSuite> pluginSuite, PluginInstance* pluginInst, AdmParameter admParameter, ReaperAPI const& api);
	bool getEnvelopeBypassed(TrackEnvelope* env, ReaperAPI const& api);
};
