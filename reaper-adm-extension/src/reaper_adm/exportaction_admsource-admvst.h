#pragma once

#include "reaperapi.h"
#include "exportaction_admsourcescontainer.h"
#include "communicators.h"
#include "exportaction_issues.h"
#include "exportaction_parameterprocessing.h"
#include "pluginsuite.h"
#include "admvstcontrol.h"
#include "helper/adm_preset_definitions_helper.h"

#include <memory>

typedef struct {
    std::shared_ptr<adm::AudioChannelFormat> audioChannelFormat;
    std::shared_ptr<adm::AudioStreamFormat> audioStreamFormat;
    std::shared_ptr<adm::AudioTrackFormat> audioTrackFormat;
    std::shared_ptr<adm::AudioTrackUid> audioTrackUid;
} AdmSubgraphElements;

class AdmVstCommunicator : public CommunicatorBase
{
public:
    AdmVstCommunicator(int samplesPort, int commandPort);
    ~AdmVstCommunicator() {}

    void updateInfo() override;
    // Existing is fine - bool nextFrameAvailable();
    // Existing is fine - bool copyNextFrame(float* buf, bool bypassAvailabilityCheck = false);

    bool gotInfo() { return infoReceived; }
    int getReportedSampleRate() override { return sampleRate; }
    int getReportedChannelCount() override { return channelCount; }
    uint16_t getReportedAdmTypeDefinition() { return admTypeDefinition; }
    uint16_t getReportedAdmPackFormatId() { return admPackFormatId; }
    uint32_t getReportedAdmPackFormatIdComplete() { return admPackFormatId + (admTypeDefinition << 16); }
    std::string getReportedAdmPackFormatIdStr() { return std::string("AP_") + intToHex(getReportedAdmPackFormatIdComplete()); }
    uint16_t getReportedAdmChannelFormatId() { return admChannelFormatId; }
    uint32_t getReportedAdmChannelFormatIdComplete() { return admChannelFormatId + (admTypeDefinition << 16); }
    std::string getReportedAdmChannelFormatIdStr() { return std::string("AC_") + intToHex(getReportedAdmChannelFormatIdComplete()); }

private:
    void infoExchange();

    bool infoReceived{ false };
    uint8_t channelCount{ 0 };
    uint32_t sampleRate{ 0 };
    uint16_t admTypeDefinition{ 0 };
    uint16_t admPackFormatId{ 0 };
    uint16_t admChannelFormatId{ 0 };
};

// Wrapper around AdmVst to make it a suitable export source
class AdmVstExporter
{
public:
    AdmVstExporter(std::shared_ptr<AdmVst> admExportVst, std::shared_ptr<adm::AudioContent> parentContent, std::shared_ptr<adm::AudioObject> audioObject, ReaperAPI const& api);
    ~AdmVstExporter();

    std::shared_ptr<AdmVst> getPlugin() {
        return admExportVst;
    }

    AdmVstCommunicator* getCommunicator(bool mustExist = false);

    bool isCandidateForExport();
    std::shared_ptr<adm::AudioObject> getAudioObject() { return audioObject; }
    std::shared_ptr<adm::AudioPackFormat> getAudioPackFormat() { return audioPackFormat; }
    std::vector<std::shared_ptr<AdmSubgraphElements>>* getAdmSubgraphs() { return &admSubgraphs; }
    std::vector<AdmAuthoringError>* getAdmAuthoringErrors() { return &admAuthoringErrors; }

    bool getRenderInProgressState();
    void setRenderInProgressState(bool state);

    int blockCount{ 0 };

private:
    void assignAdmMetadata(ReaperAPI const& api);
    std::shared_ptr<AdmSubgraphElements> newAdmSubgraph(ReaperAPI const& api);
    void newAdmPresetDefinitionReference(ReaperAPI const& api);
    void createAndAddAudioBlocks(adm::TypeDescriptor typeDescriptor, std::shared_ptr<AdmSubgraphElements> subgraph, ReaperAPI const& api);
    template<typename AudioBlockFormatType>
    void addBlockFormatsToChannelFormat(std::optional<std::vector<std::shared_ptr<AudioBlockFormatType>>> blocks, std::shared_ptr<AdmSubgraphElements> subgraph);

    std::shared_ptr<AdmVst> admExportVst;
    std::shared_ptr<adm::Document> parentDocument;
    std::shared_ptr<adm::AudioContent> parentContent;
    std::shared_ptr<adm::AudioObject> audioObject;
    std::shared_ptr<adm::AudioPackFormat> audioPackFormat;
    std::vector<std::shared_ptr<AdmSubgraphElements>> admSubgraphs;
    std::vector<AdmAuthoringError> admAuthoringErrors;

    std::shared_ptr<AdmVstCommunicator> communicator;
    bool isCommunicatorPresent();
    void obtainCommunicator();
    void releaseCommunicator();

};

class AdmVstExportSources : public IExportSources
{
    /*
    A container for handling ADM sources provided by ADM Export Source VSTs
    */
public:
    AdmVstExportSources(ReaperAPI const&api);
    ~AdmVstExportSources() {}

    bool documentRequiresProgrammeTitles() override { return true; }
    bool documentRequiresContentTitles() override { return true; }

    bool validForExport() override { return candidatesForExport.size() > 0; }
    std::vector<std::shared_ptr<AdmVst>>* getAllFoundVsts() { return &allAdmVsts; }

    int getSampleRate() override;
    int getTotalExportChannels() override;

    void setRenderInProgress(bool state) override;
    bool isFrameAvailable() override;
    bool writeNextFrameTo(float* bufferWritePointer, bool skipFrameAvailableCheck = false) override;

    std::shared_ptr<bw64::AxmlChunk> getAxmlChunk() override;
    std::shared_ptr<bw64::ChnaChunk> getChnaChunk() override;
    std::shared_ptr<adm::Document> getAdmDocument() override { return admDocument; }

    std::vector<std::string> generateExportInfoStrings() override { return infoStrings; }
    std::vector<std::string> generateExportErrorStrings() override { return errorStrings; }
    std::vector<std::string> generateExportWarningStrings() override { return warningStrings; }

    std::string getExportSourcesName() override {
        return std::string("ADM Export Source VSTs");
    }

private:
    std::shared_ptr<adm::Document> admDocument;
    std::shared_ptr<adm::AudioProgramme> admProgramme;
    std::shared_ptr<adm::AudioContent> admContent;
    std::vector<std::shared_ptr<AdmVst>> allAdmVsts{};
    std::vector<std::shared_ptr<AdmVstExporter>> candidatesForExport{};
    std::vector<std::string> infoStrings;
    std::vector<std::string> errorStrings;
    std::vector<std::string> warningStrings;

    void updateErrorsWarningsInfo(ReaperAPI const & api);
};
