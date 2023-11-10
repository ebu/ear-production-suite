#include "pluginsuite_ear.h"
#include "cartesianspeakerlayouts.h"

#include <cassert>
#include <sstream>
#include <chrono>
#include <thread>

#include "reaperapi.h"
#include "progress/importlistener.h";
#include "plugin.h"
#include "pluginregistry.h"
#include "exportaction_admsource-earvst.h" // TODO - the fact we have to include an export header here probably means some stuff is defined in the wrong place
#include "track.h"
#include "mediatrackelement.h"
#include "parameter.h"
#include "admtraits.h"
#include <adm/adm.hpp>
#include <adm/write.hpp>
#include <adm/common_definitions.hpp>
#include <bw64/bw64.hpp>
#include <speaker_setups.hpp>
#include <helper/adm_preset_definitions_helper.h>
#include <helper/container_helpers.hpp>
#include <daw_channel_count.h>

using namespace admplug;

#define TRACK_MAPPING_MIN -1
#define TRACK_MAPPING_MAX MAX_DAW_CHANNELS - 1
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
    BYPASS,
    USE_TRACK_NAME,
    INPUT_INSTANCE_ID,
    NUM_PARAMETERS
};

enum class EarDirectSpeakersParameters {
    TRACK_MAPPING = 0,
    PACKFORMAT_ID_VALUE,
    BYPASS,
    USE_TRACK_NAME,
    INPUT_INSTANCE_ID,
    NUM_PARAMETERS
};

enum class EarHoaParameters {
    TRACK_MAPPING = 0,
    PACKFORMAT_ID_VALUE,
    BYPASS,
    USE_TRACK_NAME,
    INPUT_INSTANCE_ID,
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

    auto packFormatIdParam = createPluginParameter(static_cast<int>(EarDirectSpeakersParameters::PACKFORMAT_ID_VALUE), { PACKFORMAT_ID_VALUE_MIN, PACKFORMAT_ID_VALUE_MAX });
    auto packFormatId = plugin.getParameterWithConvertToInt(*(packFormatIdParam.get()));
    assert(packFormatId.has_value());

    if (trackMapping.has_value() && *trackMapping >= 0 && packFormatId.has_value()) {
        auto pfData = AdmPresetDefinitionsHelper::getSingleton()->getPackFormatData(1, *packFormatId);
        if (pfData) {
            int trackWidth = pfData->relatedChannelFormats.size();
            for (int channelCounter = 0; channelCounter < trackWidth; channelCounter++) {
                usedValues.push_back((*trackMapping) + channelCounter);
            }
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
		auto pfData = AdmPresetDefinitionsHelper::getSingleton()->getPackFormatData(4, *packFormatId);
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

std::unique_ptr<Plugin> EARPluginSuite::createAndNamePlugin(std::string const& pluginName, TrackInstance* track, TrackElement* trackElement, int32_t routingToScene) {
    auto cbh = EARPluginCallbackHandler::getInstance();
    auto iip = EARPluginInstanceIdProvider::getInstance();
    auto mte = dynamic_cast<MediaTrackElement*>(trackElement);
    auto audioObject = mte->getRepresentedAudioObject();
    auto audioTrackUid = mte->getRepresentedAudioTrackUid();

    std::string customName;
    if(trackElement->getTakeElement()->countParentElements() > 1) {
        // Multiple plugins on this track so we need to name them seperately.
        customName = mte->getAppropriateName();
    }

    std::string xml; // Plugin Set State
    if(!customName.empty()) {
        // Need to send a state as XML
        std::string xmlElementName;
        if(pluginName == EARPluginSuite::OBJECT_METADATA_PLUGIN_NAME) xmlElementName = "ObjectsPlugin";
        if(pluginName == EARPluginSuite::DIRECTSPEAKERS_METADATA_PLUGIN_NAME) xmlElementName = "DirectSpeakersPlugin";
        if(pluginName == EARPluginSuite::HOA_METADATA_PLUGIN_NAME) xmlElementName = "HoaPlugin";
        assert(!pluginName.empty());

        xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?> <";
        xml += xmlElementName;
        xml.append(" use_track_name=\"0\" name=\"");
        xml += customName;
        xml.append("\" />");
    }

    cbh->reset();
    iip->expectRequest();
    uint32_t inputInstanceId = 0;
    auto plugin = track->createPlugin(pluginName);
    if(iip->waitForRequest(1000)) {
        auto id = iip->getLastProvidedId();
        if(id.has_value()) {
            inputInstanceId = id.value();
        }
    }
    if(!xml.empty()) {
        if(cbh->waitForPluginResponse(1000)) {
            cbh->sendData(xml);
        }
    }
    cbh->reset();

    if(!customName.empty()) {
        auto trackName = track->getName();
        if(customName != trackName) {
            track->setName("Multiple Objects");
        }
    }

    pluginToAdmMaps.push_back(PluginToAdmMap{
        audioObject ? audioObject->get<adm::AudioObjectId>().get<adm::AudioObjectIdValue>().get() : 0,
        audioTrackUid ? audioTrackUid->get<adm::AudioTrackUidId>().get<adm::AudioTrackUidIdValue>().get() : 0,
        inputInstanceId,
        routingToScene
    });

    return std::move(plugin);

}

const char* EARPluginSuite::OBJECT_METADATA_PLUGIN_NAME = "EAR Object";
const char* EARPluginSuite::DIRECTSPEAKERS_METADATA_PLUGIN_NAME = "EAR DirectSpeakers";
const char* EARPluginSuite::HOA_METADATA_PLUGIN_NAME = "EAR HOA";
const char* EARPluginSuite::SCENEMASTER_PLUGIN_NAME = "EAR Scene";
const char* EARPluginSuite::RENDERER_PLUGIN_NAME = "EAR Monitoring 0+2+0";

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

void admplug::EARPluginSuite::onProjectBuildBegin(std::shared_ptr<IADMMetaData> metadata, std::shared_ptr<ImportListener> broadcast, const ReaperAPI&)
{
    importBroadcast = broadcast;

	// Store ADM document ready for transmitting to EAR Scene
	assert(metadata);
	std::stringstream xmlStream;
	adm::writeXml(xmlStream, metadata->adm());
	originalAdmDocument = xmlStream.str();

    takesOnTracks.clear();
    pluginToAdmMaps.clear();

	sceneMasterAlreadyExisted = false;
}

void admplug::EARPluginSuite::onProjectBuildComplete(const ReaperAPI & api)
{
    importBroadcast.reset();
	assert(originalAdmDocument.length() > 0);
	if (!sceneMasterAlreadyExisted) {
		auto sceneMaster = EarSceneMasterVst(sceneMasterTrack->get(), api);
		auto samplesPort = sceneMaster.getSamplesSocketPort();
		auto commandPort = sceneMaster.getCommandSocketPort();
		auto communicator = CommunicatorRegistry::getCommunicator<EarVstCommunicator>(samplesPort, commandPort);
		communicator->sendAdm(originalAdmDocument, pluginToAdmMaps);
	}
}

void EARPluginSuite::onCreateProject(const ProjectNode&, const ReaperAPI & api)
{
	std::vector<UniqueValueAssigner::SearchCandidate> trackMappingAssignerSearches;
	trackMappingAssignerSearches.push_back(UniqueValueAssigner::SearchCandidate{ OBJECT_METADATA_PLUGIN_NAME, determineUsedObjectTrackMappingValues });
	trackMappingAssignerSearches.push_back(UniqueValueAssigner::SearchCandidate{ DIRECTSPEAKERS_METADATA_PLUGIN_NAME, determineUsedDirectSpeakersTrackMappingValues });
	trackMappingAssignerSearches.push_back(UniqueValueAssigner::SearchCandidate{ HOA_METADATA_PLUGIN_NAME, determineUsedHoaTrackMappingValues });
	trackMappingAssigner = std::make_unique<UniqueValueAssigner>(trackMappingAssignerSearches, 0, api.GetDawChannelCount() - 1, api);
	checkForExistingTracks(api);
	setTrackInsertionIndexFromSelectedMedia(api);
	if (!Track::trackPresent(sceneMasterTrack.get())) {
		sceneMasterTrack = createBusTrack(SCENEMASTER_PLUGIN_NAME, api);
		sceneMasterTrack->setName("EAR Scene Bus");
		sceneMasterTrack->disableMasterSend();
		if (!Track::trackPresent(rendererTrack.get())) {
			rendererTrack = createBusTrack(RENDERER_PLUGIN_NAME, api);
			sceneMasterTrack->routeTo(*rendererTrack, api.GetDawChannelCount());
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
	track->setChannelCount(api.GetDawChannelCount());
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

void admplug::EARPluginSuite::onCreateObjectTrack(admplug::TrackElement & trackElement, const admplug::ReaperAPI& api)
{
    auto take = trackElement.getTakeElement();
    TrackInfo trackInfo;
    auto channelCount = static_cast<int>(take->channelCount());

    if(mapHasKey(takesOnTracks, take)) {
        trackInfo = getValueFromMap(takesOnTracks, take);
        trackElement.setTrack(trackInfo.track);
    } else {
        auto mediaTrack = api.createTrackAtIndex(0, true);
        assert(mediaTrack);
        trackInfo.track = std::make_shared<TrackInstance>(mediaTrack, api);
        assert(trackMappingAssigner);
        trackInfo.routingStartChannel = trackMappingAssigner->getNextAvailableValue(channelCount);
        if(!trackInfo.routingStartChannel.has_value()) {
            if (importBroadcast) {
                importBroadcast->warning("Unable to route Object to Scene - insufficient channels.");
            }
        } else {
            assert(sceneMasterTrack);
            trackInfo.track->routeTo(*sceneMasterTrack, channelCount, 0, *trackInfo.routingStartChannel);
        }
        setInMap(takesOnTracks, take, trackInfo);
        trackElement.setTrack(trackInfo.track);
        auto mte = dynamic_cast<MediaTrackElement*>(&trackElement);
        mte->nameTrackFromElementName();
        trackInfo.track->disableMasterSend();
        trackInfo.track->setChannelCount(channelCount);
    }

    auto automationElements = trackElement.getAutomationElements();
    auto takeChannels = take->channelsOfOriginal();
    for(auto const& automationElement : automationElements) {
        auto aeChannelOfOriginal = automationElement->channel().channelOfOriginal();
        for(int chOffset = 0; chOffset < takeChannels.size(); chOffset++) {
            if(takeChannels[chOffset] == aeChannelOfOriginal) {

                int32_t routingToScene = -1;
                if(trackInfo.routingStartChannel.has_value()) {
                    routingToScene = *trackInfo.routingStartChannel + chOffset;
                }

                auto plugin = createAndNamePlugin(OBJECT_METADATA_PLUGIN_NAME, trackInfo.track.get(), &trackElement, routingToScene);

                for (auto& parameter : automatedObjectPluginParameters()) {
                    automationElement->apply(*parameter, *plugin);
                }

                if(routingToScene >= 0) {
                    plugin->setParameter(*objectTrackMappingParameter, objectTrackMappingParameter->forwardMap(routingToScene));
                }

                break;
            }
        }
    }
}

void EARPluginSuite::onCreateDirectTrack(TrackElement & trackElement, const ReaperAPI & api)
{
    auto take = trackElement.getTakeElement();
    TrackInfo trackInfo;
    auto channelCount = static_cast<int>(take->channelCount());

    if(mapHasKey(takesOnTracks, take)) {
        trackInfo = getValueFromMap(takesOnTracks, take);
        trackElement.setTrack(trackInfo.track);
    } else {
        auto mediaTrack = api.createTrackAtIndex(0, true);
        assert(mediaTrack);
        trackInfo.track = std::make_shared<TrackInstance>(mediaTrack, api);
        assert(trackMappingAssigner);
        trackInfo.routingStartChannel = trackMappingAssigner->getNextAvailableValue(channelCount);
        if(!trackInfo.routingStartChannel.has_value()) {
            if (importBroadcast) {
                importBroadcast->warning("Unable to route DirectSpeakers to Scene - insufficient channels.");
            }
        }
        setInMap(takesOnTracks, take, trackInfo);
        trackElement.setTrack(trackInfo.track);
        auto mte = dynamic_cast<MediaTrackElement*>(&trackElement);
        mte->nameTrackFromElementName();
        trackInfo.track->disableMasterSend();
        trackInfo.track->setChannelCount(channelCount);
    }


    auto automationElements = trackElement.getAutomationElements();
    if(automationElements.size() > 0) {
        auto channel = automationElements.front()->channel();
        auto packFormat = channel.packFormat();
        auto packFormatIdValue = packFormat->get<adm::AudioPackFormatId>().get<adm::AudioPackFormatIdValue>().get();

        auto defs = AdmPresetDefinitionsHelper::getSingleton();
        auto pfData = defs->getPackFormatData(1, packFormatIdValue);
        if (!pfData) {
            // Could be cart pf
            auto cartLayout = getCartLayout(*packFormat);
            if (cartLayout) {
                auto altPfIdStr = getMappedCommonPackId(*cartLayout);
                auto altPfId = adm::parseAudioPackFormatId(altPfIdStr);
                auto altPfIdValue = altPfId.get<adm::AudioPackFormatIdValue>().get();
                pfData = defs->getPackFormatData(1, altPfIdValue);
                if (pfData) {
                    packFormatIdValue = altPfIdValue;
                }
            }
        }
        if (!pfData) {
            // Some third-party tools do not use consistent PF IDs for their custom definitions.
            // Try lookup by channels.
            pfData = defs->getPackFormatDataByMatchingChannels(packFormat);
            if (pfData) {
                packFormatIdValue = pfData->idValue;
            }
        }

        if(pfData) {
            int32_t routingToScene = -1;
            if(trackInfo.routingStartChannel.has_value()) {
                routingToScene = *trackInfo.routingStartChannel;
            }

            auto plugin = createAndNamePlugin(DIRECTSPEAKERS_METADATA_PLUGIN_NAME, trackInfo.track.get(), &trackElement, routingToScene);
            plugin->setParameter(*directPackFormatIdValueParameter, directPackFormatIdValueParameter->forwardMap(packFormatIdValue));

            if(routingToScene >= 0) {
                plugin->setParameter(*directSpeakersTrackMappingParameter, directSpeakersTrackMappingParameter->forwardMap(routingToScene));
                trackInfo.track->routeTo(*sceneMasterTrack, channelCount, 0, routingToScene);
            }

        } else {
            if (importBroadcast) {
                importBroadcast->warning("Unsupported DirectSpeakers audioPackFormat.");
            }
        }
    }
}

void EARPluginSuite::onCreateGroup(TrackElement & trackElement, const ReaperAPI&)
{
	assert(false); // This should never be called, because we've said we don't require groups for this plugin suite!
}

void EARPluginSuite::onCreateHoaTrack(TrackElement &trackElement, const ReaperAPI &api)
{
    auto take = trackElement.getTakeElement();
    TrackInfo trackInfo;
    auto channelCount = static_cast<int>(take->channelCount());

    if(mapHasKey(takesOnTracks, take)) {
        trackInfo = getValueFromMap(takesOnTracks, take);
        trackElement.setTrack(trackInfo.track);
    } else {
        auto mediaTrack = api.createTrackAtIndex(0, true);
        assert(mediaTrack);
        trackInfo.track = std::make_shared<TrackInstance>(mediaTrack, api);
        assert(trackMappingAssigner);
        trackInfo.routingStartChannel = trackMappingAssigner->getNextAvailableValue(channelCount);
        if(!trackInfo.routingStartChannel.has_value()) {
            if (importBroadcast) {
                importBroadcast->warning("Unable to route HOA to Scene - insufficient channels.");
            }
        }
        setInMap(takesOnTracks, take, trackInfo);
        trackElement.setTrack(trackInfo.track);
        auto mte = dynamic_cast<MediaTrackElement*>(&trackElement);
        mte->nameTrackFromElementName();
        trackInfo.track->disableMasterSend();
        trackInfo.track->setChannelCount(channelCount);
    }

    auto automationElements = trackElement.getAutomationElements();
    if(automationElements.size() > 0) {
        auto channel = automationElements.front()->channel();
        auto packFormat = channel.packFormat();
        assert(packFormat);

        if(packFormat) {
            auto packFormatId = std::stoi(adm::formatId(packFormat->get<adm::AudioPackFormatId>()).substr(7, 4));

            int32_t routingToScene = -1;
            if(trackInfo.routingStartChannel.has_value()) {
                routingToScene = *trackInfo.routingStartChannel;
            }

            auto plugin = createAndNamePlugin(HOA_METADATA_PLUGIN_NAME, trackInfo.track.get(), &trackElement, routingToScene);
            plugin->setParameter(*hoaPackFormatIdValueParameter, hoaPackFormatIdValueParameter->forwardMap(packFormatId));

            if(routingToScene >= 0) {
                plugin->setParameter(*directSpeakersTrackMappingParameter, directSpeakersTrackMappingParameter->forwardMap(routingToScene));
                trackInfo.track->routeTo(*sceneMasterTrack, channelCount, 0, routingToScene);
            }

        } else {
            if (importBroadcast) {
                importBroadcast->warning("Unsupported HOA audioPackFormat.");
            }
        }
    }
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

std::shared_ptr<EARPluginCallbackHandler> admplug::EARPluginCallbackHandler::getInstance()
{
    static auto instance = std::make_shared<EARPluginCallbackHandler>();
    return instance;
}

void admplug::EARPluginCallbackHandler::reset()
{
    std::lock_guard<std::mutex> lock(activeCallbackMutex);
    activeCallback.reset();
}

bool admplug::EARPluginCallbackHandler::waitForPluginResponse(uint16_t maxMs)
{
    std::chrono::milliseconds dur(maxMs);
    auto start = std::chrono::system_clock::now();
    auto end = start + dur;
    const std::chrono::milliseconds intervalMs(10);

    while((std::chrono::system_clock::now() + intervalMs) < end) {
        std::this_thread::sleep_for(intervalMs);
        std::lock_guard<std::mutex> lock(activeCallbackMutex);
        if(activeCallback.has_value()) {
            return true;
        }
    }
    return false;
}

bool admplug::EARPluginCallbackHandler::sendData(std::string const & xmlState)
{
    std::lock_guard<std::mutex> lock(activeCallbackMutex);
    if(activeCallback.has_value()) {
        activeCallback.value()(xmlState);
        return true;
    }
    return false;
}

void admplug::EARPluginCallbackHandler::setPluginCallback(std::function<void(std::string const&)> callback)
{
    std::lock_guard<std::mutex> lock(activeCallbackMutex);
    activeCallback = callback;
}

admplug::EARPluginCallbackHandler::EARPluginCallbackHandler()
{
}

admplug::EARPluginCallbackHandler::~EARPluginCallbackHandler()
{
}

admplug::EARPluginInstanceIdProvider::EARPluginInstanceIdProvider()
{
}

admplug::EARPluginInstanceIdProvider::~EARPluginInstanceIdProvider()
{
}

std::shared_ptr<EARPluginInstanceIdProvider> admplug::EARPluginInstanceIdProvider::getInstance()
{
    static auto instance = std::make_shared<EARPluginInstanceIdProvider>();
    return instance;
}

void admplug::EARPluginInstanceIdProvider::expectRequest()
{
    awaitingRequest = true;
}

bool admplug::EARPluginInstanceIdProvider::waitForRequest(uint16_t maxMs)
{
    std::chrono::milliseconds dur(maxMs);
    auto start = std::chrono::system_clock::now();
    auto end = start + dur;
    const std::chrono::milliseconds intervalMs(10);

    while((std::chrono::system_clock::now() + intervalMs) < end) {
        std::this_thread::sleep_for(intervalMs);
        if(!awaitingRequest) {
            return true;
        }
    }
    return false;
}

std::optional<uint32_t> admplug::EARPluginInstanceIdProvider::getLastProvidedId()
{
    std::lock_guard<std::mutex> lock(lastProvidedIdMutex);
    return lastProvidedId;
}

uint32_t admplug::EARPluginInstanceIdProvider::getNextAvailableId()
{
    std::lock_guard<std::mutex> lock(lastProvidedIdMutex);
    if(lastProvidedId.has_value()) {
        return lastProvidedId.value() + 1;
    }
    return 1; // Start at ID 1 - makes it easier to spot unset IDs (val would be 0)
}

uint32_t admplug::EARPluginInstanceIdProvider::provideId()
{
    auto id = getNextAvailableId();
    std::lock_guard<std::mutex> lock(lastProvidedIdMutex);
    lastProvidedId = id;
    awaitingRequest = false;
    return id;
}
