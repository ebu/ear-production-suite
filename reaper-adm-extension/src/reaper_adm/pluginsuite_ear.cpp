#include "pluginsuite_ear.h"
#include "cartesianspeakerlayouts.h"

#include <cassert>
#include <sstream>

#include "reaperapi.h"
#include "plugin.h"
#include "pluginregistry.h"
#include "exportaction_admsource-earvst.h" // TODO - the fact we have to include an export header here probably means some stuff is defined in the wrong place
#include "track.h"
#include "parameter.h"
#include "admtraits.h"
#include <adm/adm.hpp>
#include <adm/write.hpp>
#include <adm/common_definitions.hpp>
#include <bw64/bw64.hpp>
#include <speaker_setups.hpp>
#include <helper/common_definition_helper.h>

using namespace admplug;

#define TRACK_MAPPING_MIN -1
#define TRACK_MAPPING_MAX 63
#define PACKFORMAT_ID_VALUE_MIN 0x0
#define PACKFORMAT_ID_VALUE_MAX 0xFFFF

namespace {

enum class EarObjectParameters {
    TRACK_MAPPING = 0,
    GAIN,
    AZIMUTH,
    ELEVATION,
    DISTANCE,
    LINK_SIZE, // Irrelevant
    SIZE, // Irrelevant
    WIDTH,
    HEIGHT,
    DEPTH,
    DIFFUSE,
    DIVERGENCE, // Since the next 2 params are not supported by libadm yet, this is irrelevant
    DIVERGENCE_FACTOR, // Not supported by libadm yet
    DIVERGENCE_RANGE, // Not supported by libadm yet
    NUM_PARAMETERS
};

enum class EarDirectSpeakersParameters {
    TRACK_MAPPING = 0,
    PACKFORMAT_ID_VALUE,
    NUM_PARAMETERS
};

enum class EarHoaParameters {
    TRACK_MAPPING = 0,
    PACKFORMAT_ID_VALUE,
    NUM_PARAMETERS
};

std::vector<std::unique_ptr<PluginParameter>> createAutomatedObjectPluginParameters()
{
    std::vector<std::unique_ptr<PluginParameter>> parameters;
    parameters.push_back(createPluginParameter(static_cast<int>(EarObjectParameters::AZIMUTH),
                                               AdmParameter::OBJECT_AZIMUTH,
                                               { -180.0, 180.0 }));
    parameters.push_back(createPluginParameter(static_cast<int>(EarObjectParameters::DISTANCE),
                                               AdmParameter::OBJECT_DISTANCE,
                                               { 0.0, 1.0 }));
    parameters.push_back(createPluginParameter(static_cast<int>(EarObjectParameters::ELEVATION),
                                               AdmParameter::OBJECT_ELEVATION,
                                               { -90.0, 90.0 }));
    parameters.push_back(createPluginParameter(static_cast<int>(EarObjectParameters::GAIN),
                                               AdmParameter::OBJECT_GAIN,
                                               map::linearToDb({ -100.0, 6.0 })));
   parameters.push_back(createPluginParameter(static_cast<int>(EarObjectParameters::HEIGHT),
                                               AdmParameter::OBJECT_HEIGHT,
                                               { 0.0, 90.0 }));
    parameters.push_back(createPluginParameter(static_cast<int>(EarObjectParameters::WIDTH),
                                               AdmParameter::OBJECT_WIDTH,
                                               { 0.0, 360.0 }));
    parameters.push_back(createPluginParameter(static_cast<int>(EarObjectParameters::DEPTH),
                                               AdmParameter::OBJECT_DEPTH,
                                               { 0.0, 1.0 }));
    parameters.push_back(createPluginParameter(static_cast<int>(EarObjectParameters::DIFFUSE),
                                               AdmParameter::OBJECT_DIFFUSE,
                                               { 0.0, 1.0 }));
//        parameters.push_back(createPluginParameter(static_cast<int>(EarObjectParameters::DIVERGENCE),
//                                                   AdmParameter::OBJECT_DIVERGENCE,
//                                                   {0.0, 1.0}));
//        parameters.push_back(createPluginParameter(static_cast<int>(EarObjectParameters::DIVERGENCE_AZIMUTH_RANGE),
//                                                   AdmParameter::OBJECT_DIVERGENCE_AZIMUTH_RANGE,
//                                                   {0.0, 180.0}));
    return parameters;
}

std::vector<std::unique_ptr<TrackParameter>> createTrackParameters() {
    std::vector<std::unique_ptr<TrackParameter>> parameters;
    return parameters;
}

std::vector<int> determineUsedObjectTrackMappingValues(PluginInstance& plugin) {
	auto param = createPluginParameter(static_cast<int>(EarObjectParameters::TRACK_MAPPING), { TRACK_MAPPING_MIN, TRACK_MAPPING_MAX });
	auto trackMapping = plugin.getParameterWithConvertToInt(*(param.get()));
	assert(trackMapping.has_value());
	if (trackMapping.has_value()) {
		return std::vector<int>{ *trackMapping };
	}
	return std::vector<int>{};
}

std::vector<int> determineUsedDirectSpeakersTrackMappingValues(PluginInstance& plugin) {
    std::vector<int> usedValues{};

    auto trackMappingParam = createPluginParameter(static_cast<int>(EarDirectSpeakersParameters::TRACK_MAPPING), { TRACK_MAPPING_MIN, TRACK_MAPPING_MAX });
    auto trackMapping = plugin.getParameterWithConvertToInt(*(trackMappingParam.get()));
    assert(trackMapping.has_value());

    auto packFormatIdValueParam = createPluginParameter(static_cast<int>(EarDirectSpeakersParameters::PACKFORMAT_ID_VALUE), { PACKFORMAT_ID_VALUE_MIN, PACKFORMAT_ID_VALUE_MAX });
    auto packFormatIdValue = plugin.getParameterWithConvertToInt(*(packFormatIdValueParam.get()));
    assert(packFormatIdValue.has_value());

    int trackWidth = 1; // Track mapping is single channel by default.
    if(packFormatIdValue.has_value()) {
        auto speakerLayoutIndex = ear::plugin::getIndexFromPackFormatIdValue(*packFormatIdValue);
        if(speakerLayoutIndex >= 0) {
            trackWidth = ear::plugin::SPEAKER_SETUPS[speakerLayoutIndex].speakers.size();
        }
    }

    if(trackMapping.has_value() && *trackMapping >= 0) {
        for(int channelCounter = 0; channelCounter < trackWidth; channelCounter++) {
            usedValues.push_back((*trackMapping) + channelCounter);
        }
    }

    return usedValues;
}

std::vector<int> determineUsedHoaTrackMappingValues(PluginInstance& plugin) {
	std::vector<int> usedValues{};

	auto param = createPluginParameter(static_cast<int>(EarObjectParameters::TRACK_MAPPING), { TRACK_MAPPING_MIN, TRACK_MAPPING_MAX });
	auto trackMapping = plugin.getParameterWithConvertToInt(*(param.get()));
	assert(trackMapping.has_value());

	auto packFormatIdParam = createPluginParameter(static_cast<int>(EarHoaParameters::PACKFORMAT_ID_VALUE), { PACKFORMAT_ID_VALUE_MIN, PACKFORMAT_ID_VALUE_MAX });
	auto packFormatId = plugin.getParameterWithConvertToInt(*(packFormatIdParam.get()));
	assert(packFormatId.has_value());

	if (trackMapping.has_value() && *trackMapping >= 0 && packFormatId.has_value()) {
		auto pfData = AdmCommonDefinitionHelper::getSingleton()->getPackFormatData(4, *packFormatId);
		if(pfData) {
			int trackWidth = pfData->relatedChannelFormats.size();
			for (int channelCounter = 0; channelCounter < trackWidth; channelCounter++) {
				usedValues.push_back((*trackMapping) + channelCounter);
			}
		}
	}

	return usedValues;
}

}

const char* EARPluginSuite::OBJECT_METADATA_PLUGIN_NAME = "EAR Object";
const char* EARPluginSuite::DIRECTSPEAKERS_METADATA_PLUGIN_NAME = "EAR DirectSpeakers";
const char* EARPluginSuite::HOA_METADATA_PLUGIN_NAME = "EAR HOA";
const char* EARPluginSuite::SCENEMASTER_PLUGIN_NAME = "EAR Scene";
const char* EARPluginSuite::RENDERER_PLUGIN_NAME = "EAR Monitoring 0+2+0";
const int EARPluginSuite::MAX_CHANNEL_COUNT = 64;

bool EARPluginSuite::registered = PluginRegistry::getInstance()->registerSupportedPluginSuite("EAR", std::make_shared<EARPluginSuite>());

EARPluginSuite::EARPluginSuite() :
	objectTrackMappingParameter{ createPluginParameter(static_cast<int>(EarObjectParameters::TRACK_MAPPING), {TRACK_MAPPING_MIN, TRACK_MAPPING_MAX}) },
	directSpeakersTrackMappingParameter{ createPluginParameter(static_cast<int>(EarDirectSpeakersParameters::TRACK_MAPPING), {TRACK_MAPPING_MIN, TRACK_MAPPING_MAX}) },
	directPackFormatIdValueParameter{ createPluginParameter(static_cast<int>(EarDirectSpeakersParameters::PACKFORMAT_ID_VALUE), {PACKFORMAT_ID_VALUE_MIN, PACKFORMAT_ID_VALUE_MAX}) },
	hoaTrackMappingParameter{ createPluginParameter(static_cast<int>(EarHoaParameters::TRACK_MAPPING), {TRACK_MAPPING_MIN, TRACK_MAPPING_MAX}) },
	hoaPackFormatIdValueParameter{ createPluginParameter(static_cast<int>(EarHoaParameters::PACKFORMAT_ID_VALUE), {PACKFORMAT_ID_VALUE_MIN, PACKFORMAT_ID_VALUE_MAX}) }
{
}

EARPluginSuite::~EARPluginSuite() = default;

void admplug::EARPluginSuite::onProjectBuildBegin(std::shared_ptr<IADMMetaData> metadata, const ReaperAPI&)
{
	// Prepare trackMapping vectors for population
	trackMappingToAtu = std::vector<uint32_t>(MAX_CHANNEL_COUNT, 0x00000000); // ATUID of 0x00000000 = not used
	trackMappingToAo = std::vector<uint32_t>(MAX_CHANNEL_COUNT, 0x00000000);

	// Store ADM document ready for transmitting to EAR Scene
	assert(metadata);
	std::stringstream xmlStream;
	adm::writeXml(xmlStream, metadata->adm());
	originalAdmDocument = xmlStream.str();

	sceneMasterAlreadyExisted = false;
}

void admplug::EARPluginSuite::onProjectBuildComplete(const ReaperAPI & api)
{
	assert(trackMappingToAtu.size() == 64);
	assert(trackMappingToAo.size() == 64);
	assert(originalAdmDocument.length() > 0);

	if (!sceneMasterAlreadyExisted) {
		auto sceneMaster = EarSceneMasterVst(sceneMasterTrack->get(), api);
		auto samplesPort = sceneMaster.getSamplesSocketPort();
		auto commandPort = sceneMaster.getCommandSocketPort();
		auto communicator = CommunicatorRegistry::getCommunicator<EarVstCommunicator>(samplesPort, commandPort);
		communicator->sendAdmAndMappings(originalAdmDocument, trackMappingToAtu, trackMappingToAo);
	}
}

void EARPluginSuite::onCreateProject(const ProjectNode&, const ReaperAPI & api)
{
	std::vector<UniqueValueAssigner::SearchCandidate> trackMappingAssignerSearches;
	trackMappingAssignerSearches.push_back(UniqueValueAssigner::SearchCandidate{ OBJECT_METADATA_PLUGIN_NAME, determineUsedObjectTrackMappingValues });
	trackMappingAssignerSearches.push_back(UniqueValueAssigner::SearchCandidate{ DIRECTSPEAKERS_METADATA_PLUGIN_NAME, determineUsedDirectSpeakersTrackMappingValues });
	trackMappingAssignerSearches.push_back(UniqueValueAssigner::SearchCandidate{ HOA_METADATA_PLUGIN_NAME, determineUsedHoaTrackMappingValues });
	trackMappingAssigner = std::make_unique<UniqueValueAssigner>(trackMappingAssignerSearches, 0, TRACK_MAPPING_MAX, api);
	checkForExistingTracks(api);
	setTrackInsertionIndexFromSelectedMedia(api);
	if (!Track::trackPresent(sceneMasterTrack.get())) {
		sceneMasterTrack = createBusTrack(SCENEMASTER_PLUGIN_NAME, api);
		sceneMasterTrack->setName("EAR Scene Bus");
		sceneMasterTrack->disableMasterSend();
		if (!Track::trackPresent(rendererTrack.get())) {
			rendererTrack = createBusTrack(RENDERER_PLUGIN_NAME, api);
			sceneMasterTrack->routeTo(*rendererTrack, MAX_CHANNEL_COUNT);
			rendererTrack->setName("EAR Monitor Bus");
		}
	}
	else {
		sceneMasterAlreadyExisted = true;
	}
}

std::unique_ptr<Track> EARPluginSuite::createBusTrack(std::string pluginName, ReaperAPI const& api)
{
	auto track = api.createTrack();
	track->moveToBefore(trackInsertionIndex++);
	track->hideFromTrackControlPanel();
	track->createPlugin(pluginName);
	track->setChannelCount(MAX_CHANNEL_COUNT);
	return track;
}

void admplug::EARPluginSuite::checkForExistingTracks(const ReaperAPI & api)
{
	sceneMasterTrack = api.firstTrackWithPluginNamed(SCENEMASTER_PLUGIN_NAME);
	rendererTrack = api.firstTrackWithPluginNamed(RENDERER_PLUGIN_NAME);
}

void EARPluginSuite::setTrackInsertionIndexFromSelectedMedia(ReaperAPI const& api)
{
	trackInsertionIndex = api.getTrackIndexOfSelectedMediaItem();
	if (trackInsertionIndex == -1) {
		trackInsertionIndex = 1;
	}
}

void admplug::EARPluginSuite::onCreateObjectTrack(const admplug::TrackElement & trackElement, const admplug::ReaperAPI&)
{
	auto track = trackElement.getTrack();
	track->disableMasterSend();
}

void EARPluginSuite::onCreateDirectTrack(const TrackElement & trackElement, const ReaperAPI & api)
{
	// Can't configure plugin or routing just yet because we need the channel count for the pack format in onDirectSpeakersAutomation
	auto track = trackElement.getTrack();
	track->disableMasterSend();
}

void EARPluginSuite::onCreateGroup(const TrackElement & trackElement, const ReaperAPI&)
{
	assert(false); // This should never be called, because we've said we don't require groups for this plugin suite!
}

void EARPluginSuite::onObjectAutomation(const ObjectAutomation & automationElement, const ReaperAPI&)
{
	auto track = automationElement.getTrack();
	auto channelName = automationElement.channel().name();
	if (!channelName.empty()) {
		track->setName(channelName);
	}

	auto plugin = track->createPlugin(OBJECT_METADATA_PLUGIN_NAME);

	if (plugin) {
		for (auto& parameter : automatedObjectPluginParameters()) {
			automationElement.apply(*parameter, *plugin);
		}

		for (auto& parameter : trackParameters()) {
			automationElement.apply(*parameter, *track);
		}

		assert(trackMappingAssigner);
		auto trackMapping = trackMappingAssigner->getNextAvailableValue();
		if (trackMapping.has_value()) {
			plugin->setParameter(*objectTrackMappingParameter, objectTrackMappingParameter->forwardMap(*trackMapping));
			assert(sceneMasterTrack);
			track->routeTo(*sceneMasterTrack, 1, 0, *trackMapping);

			// Store mapping to send to EAR Scene
			auto takeChannel = automationElement.channel();
			auto trackUidVal = takeChannel.trackUid() ? getIdValueAsInt(*(takeChannel.trackUid())) : 0;
			uint32_t audioObjectIdVal = 0;
			if(auto ao = automationElement.channel().object()) {
				audioObjectIdVal = ao->get<adm::AudioObjectId>().get<adm::AudioObjectIdValue>().get();
			}
			trackMappingToAtu[*trackMapping] = trackUidVal;
			trackMappingToAo[*trackMapping] = audioObjectIdVal;
		}
		else {
			//TODO - need to tell user - no free channels
		}
	}
}

void EARPluginSuite::onDirectSpeakersAutomation(const DirectSpeakersAutomation & automationElement, const ReaperAPI&)
{
    // Can only do this in onDirectSpeakersAutomation because we need the packformat and a channelformats first blockformat
    // NOTE: This will run for every leg, so don't duplicate effort!
    auto track = automationElement.getTrack();
    auto plugin = track->getPlugin(DIRECTSPEAKERS_METADATA_PLUGIN_NAME);
    auto packFormat = automationElement.channel().packFormat();
    assert(packFormat);

    if(!plugin){ // Not processed yet
        auto speakerLayoutIndex = ear::plugin::getIndexFromPackFormatId(adm::formatId(packFormat->get<adm::AudioPackFormatId>()));

        if(speakerLayoutIndex < 0) {
            auto cartLayout = getCartLayout(*packFormat);
            if(cartLayout) {
                speakerLayoutIndex = ear::plugin::getIndexFromPackFormatId(getMappedCommonPackId(*cartLayout));
            }
        }

        if(speakerLayoutIndex >= 0) {
            plugin = track->createPlugin(DIRECTSPEAKERS_METADATA_PLUGIN_NAME);
            auto take = automationElement.parentTake();
            auto channelCountTake = static_cast<int>(take->trackUidCount());
            auto channelCountPackFormat = packFormat->getReferences<adm::AudioChannelFormat>().size();
            assert(channelCountTake == channelCountPackFormat); // Possibly not the same? Need to figure out how to deal with this
            auto channelCount = channelCountTake;
            track->setChannelCount(channelCount);
            auto packFormatIdValue = ear::plugin::SPEAKER_SETUPS[speakerLayoutIndex].packFormatIdValue;
            plugin->setParameter(*directPackFormatIdValueParameter, directPackFormatIdValueParameter->forwardMap(packFormatIdValue));

            assert(trackMappingAssigner);
            auto trackMapping = trackMappingAssigner->getNextAvailableValue(channelCount);
            if(trackMapping.has_value()) {
                plugin->setParameter(*directSpeakersTrackMappingParameter, directSpeakersTrackMappingParameter->forwardMap(*trackMapping));
                track->routeTo(*sceneMasterTrack, channelCount, 0, *trackMapping);

                uint32_t audioObjectIdVal = 0;
                if(auto ao = automationElement.channel().object()) {
                    audioObjectIdVal = ao->get<adm::AudioObjectId>().get<adm::AudioObjectIdValue>().get();
                }

                // Store mapping to send to EAR Scene - these should be ordered, so we can assume we just step through them
                auto trackNumber = *trackMapping;
                trackMappingToAo[trackNumber] = audioObjectIdVal;
                for(auto const& trackUid : take->trackUids()) {
                    auto trackUidVal = getIdValueAsInt(*trackUid);
                    trackMappingToAtu[trackNumber] = trackUidVal;
                    trackNumber++;
                }

            } else {
                //TODO - need to tell user - no free track mappings
            }
        } else {
            // TODO - warn user - can't support this directspeaker pack format
        }
    }
}

void EARPluginSuite::onHoaAutomation(const HoaAutomation & automationElement, const ReaperAPI & api) {
	// Can only do this in onHoaAutomation because we need the packformat and a channelformats first blockformat
	// NOTE: This will run for every leg, so don't duplicate effort!
	auto track = automationElement.getTrack();
	auto plugin = track->getPlugin(HOA_METADATA_PLUGIN_NAME);
	auto packFormat = automationElement.channel().packFormat();
	auto packFormatId = std::stoi(adm::formatId(packFormat->get<adm::AudioPackFormatId>()).substr(7, 4));
	assert(packFormat);

	if (!plugin) {
		plugin = track->createPlugin(HOA_METADATA_PLUGIN_NAME);

		auto channelCount = 1;
		auto pfData = AdmCommonDefinitionHelper::getSingleton()->getPackFormatData(4, packFormatId);
		if(pfData) {
			channelCount = pfData->relatedChannelFormats.size();
		}

		track->setChannelCount(channelCount);
		plugin->setParameter(*hoaPackFormatIdValueParameter, hoaPackFormatIdValueParameter->forwardMap(packFormatId));

		assert(trackMappingAssigner);
		auto trackMapping = trackMappingAssigner->getNextAvailableValue(channelCount);
		if (trackMapping.has_value()) {
			plugin->setParameter(*hoaTrackMappingParameter, hoaTrackMappingParameter->forwardMap(*trackMapping));
			track->routeTo(*sceneMasterTrack, channelCount, 0, *trackMapping);

			uint32_t audioObjectIdVal = 0;
			if(auto ao = automationElement.channel().object()) {
				audioObjectIdVal = ao->get<adm::AudioObjectId>().get<adm::AudioObjectIdValue>().get();
			}

			// Store mapping to send to EAR Scene - these should be ordered, so we can assume we just step through them
			auto trackNumber = *trackMapping;
            auto take = automationElement.parentTake();
			trackMappingToAo[trackNumber] = audioObjectIdVal;
            for(auto const& trackUid : take->trackUids()) {
				auto trackUidVal = getIdValueAsInt(*trackUid);
				trackMappingToAtu[trackNumber] = trackUidVal;
				trackNumber++;
			}
		}
		else {
			//TODO - need to tell user - no free track mappings
		}
	}
}

void EARPluginSuite::onCreateHoaTrack(const TrackElement &trackNode, const ReaperAPI &api){
    auto track = trackNode.getTrack();
    track->disableMasterSend();
}

bool EARPluginSuite::pluginSuiteUsable(const ReaperAPI &api)
{
	auto registry = PluginRegistry::getInstance();
	return registry->checkPluginsAvailable(
		{
			OBJECT_METADATA_PLUGIN_NAME,
			DIRECTSPEAKERS_METADATA_PLUGIN_NAME,
			HOA_METADATA_PLUGIN_NAME,
			SCENEMASTER_PLUGIN_NAME,
			RENDERER_PLUGIN_NAME
		},
		api);
}

bool admplug::EARPluginSuite::representAdmStructureWithGroups(ReaperAPI const& api)
{
	return false; // Scene master does this!
}

std::vector<std::unique_ptr<PluginParameter>> const& EARPluginSuite::automatedObjectPluginParameters()
{
	auto static parameters = createAutomatedObjectPluginParameters();
	return parameters;
}

std::vector<std::unique_ptr<TrackParameter>> const& EARPluginSuite::trackParameters() {
	auto static parameters = createTrackParameters();
	return parameters;
}

bool EARPluginSuite::applyFXPreset(const HoaAutomation&, const ReaperAPI & api) {
	return true;
}

PluginParameter* admplug::EARPluginSuite::getPluginParameterFor(AdmParameter admParameter)
{
	for (auto& param : automatedObjectPluginParameters()) {
		if (param->admParameter() == admParameter) {
			return param.get();
		}
	}
	return nullptr;
}

Parameter* admplug::EARPluginSuite::getParameterFor(AdmParameter admParameter)
{
	return getPluginParameterFor(admParameter);
}

std::vector<ADMChannel>
EARPluginSuite::reorderAndFilter(const std::vector<ADMChannel> &channels,
	const ReaperAPI & api) {
	auto pack = channels.front().packFormat();
	auto modifiedChannels{ channels };

	if (auto cartLayout = getCartLayout(*pack)) {
		// e.g. 7.0 is not in common definition, but 7.1 is so
		// add in an LFE channel filled with silence in these
		// cases.
		auto silentIndices = silentTrackIndicesFor(*cartLayout);
		for (auto index : silentIndices) {
			auto insertionPos = modifiedChannels.cbegin();
			std::advance(insertionPos, index);
			modifiedChannels.insert(insertionPos, ADMChannel{ nullptr, nullptr, nullptr, nullptr });
		}
	}

	return PluginSuite::reorderAndFilter(modifiedChannels, api);
}
