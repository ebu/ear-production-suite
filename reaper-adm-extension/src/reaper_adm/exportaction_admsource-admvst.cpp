#include "exportaction_admsource-admvst.h"

#include "pluginregistry.h"
#include <version/eps_version.h>
#include <helper/adm_preset_definitions_helper.h>

#include <adm/adm.hpp>
#include <adm/utilities/id_assignment.hpp>
#include <adm/write.hpp>
#include <adm/common_definitions.hpp>
#include <optional>

namespace {
std::vector<adm::TypeDescriptor> getAdmTypeDefinitionsExcluding(adm::TypeDescriptor exclude) {
	std::vector<adm::TypeDescriptor> otherTypes;
	if (exclude != adm::TypeDefinition::OBJECTS) otherTypes.push_back(adm::TypeDefinition::OBJECTS);
	if (exclude != adm::TypeDefinition::DIRECT_SPEAKERS) otherTypes.push_back(adm::TypeDefinition::DIRECT_SPEAKERS);
	if (exclude != adm::TypeDefinition::HOA) otherTypes.push_back(adm::TypeDefinition::HOA);
	if (exclude != adm::TypeDefinition::MATRIX) otherTypes.push_back(adm::TypeDefinition::MATRIX);
	if (exclude != adm::TypeDefinition::BINAURAL) otherTypes.push_back(adm::TypeDefinition::BINAURAL);
	return otherTypes;
}
}

AdmVstExportSources::AdmVstExportSources(ReaperAPI const& api) : IExportSources(api)
{
	admDocument = adm::Document::create();
	admDocument->set(adm::Version("ITU-R_BS.2076-2"));
	admProgramme = adm::AudioProgramme::create(adm::AudioProgrammeName("Programme"));
	admContent = adm::AudioContent::create(adm::AudioContentName("Content"));

	adm::addCommonDefinitionsTo(admDocument);
	admDocument->add(admProgramme);
	admProgramme->addReference(admContent);

	MediaTrack* trk;
	int numTracks = api.CountTracks(nullptr);

	for (int trackNum = 0; trackNum < numTracks; trackNum++) {

		trk = api.GetTrack(nullptr, trackNum);
		if (trk) {

			auto fxPosVec = AdmVst::trackAdmVstIndexes(api, trk);
			for (int fxPos : fxPosVec) {
				auto admExportVst = std::make_shared<AdmVst>(trk, fxPos, api);
				allAdmVsts.push_back(admExportVst);

				if (AdmVst::isCandidateForExport(admExportVst)) {

					char aoName[100];
					std::string aoNameStr = "Audio Object";
					if (api.GetTrackName(trk, aoName, 100)) {
						aoNameStr = aoName;
					}

					auto bounds = api.getTrackAudioBounds(trk, true); // True = ignore before zero - we don't do sub-zero bounds

					auto audioObject = adm::AudioObject::create(adm::AudioObjectName(aoNameStr));
					if (bounds.has_value()) {
						audioObject->set(adm::Start{ toNs((*bounds).first) });
						audioObject->set(adm::Duration{ toNs((*bounds).second - (*bounds).first) });
					}

					admContent->addReference(audioObject);
					// Construction generates ADM
					auto candidate = std::make_shared<AdmVstExporter>(admExportVst, admContent, audioObject, api);
					candidatesForExport.push_back(candidate);
				}
			}
		}
	}

	adm::reassignIds(admDocument); // Must be done before allowing to generate AXML/CHNA to align ID's

	updateErrorsWarningsInfo(api);
}

void AdmVstExportSources::updateErrorsWarningsInfo(ReaperAPI const& api)
{
	std::string op;
	op.append(std::to_string(candidatesForExport.size()));
	op.append(" (of ");
	op.append(std::to_string(allAdmVsts.size()));
	op.append(") instances of ");
	op.append(getExportSourcesName());
	op.append(" will export.");
	infoStrings.push_back(op);

	int notIncluded = 0;
	int offlineOrBypassed = 0;
	int knownSampleRate = 0;
	bool sampleRatesMatch = true;

	for (auto admVst : allAdmVsts) {
		int vstSampleRate = admVst->getSampleRate();
		int vstChannelCount = admVst->getChannelCount();

		if (!admVst->getIncludeInRenderState()) notIncluded++;

		if (admVst->isBypassed() || admVst->isPluginOffline()) {
			offlineOrBypassed++;
		}
		else {
			if (admVst->getIncludeInRenderState()) {
				if (vstSampleRate == 0) {
					std::string msg("VST on track '");
					char buf[32];
					api.GetTrackName(admVst->getTrackInstance().get(), buf, 32);
					msg.append(buf);
					msg.append("' is not reporting sample rate");
					warningStrings.push_back(msg);
				}
				if (admVst->getAdmTypeDefinition() == 0) {
					std::string msg("VST on track '");
					char buf[32];
					api.GetTrackName(admVst->getTrackInstance().get(), buf, 32);
					msg.append(buf);
					msg.append("' has TypeDefinition 'Undefined'");
					warningStrings.push_back(msg);
				}
				if (vstChannelCount == 0) {
					std::string msg("VST on track '");
					char buf[32];
					api.GetTrackName(admVst->getTrackInstance().get(), buf, 32);
					msg.append(buf);
					msg.append("' is reporting that it will export zero channels of audio");
					warningStrings.push_back(msg);
				}
				if (vstChannelCount > admVst->getTrackInstance().getChannelCount()) {
					std::string msg("Track '");
					char buf[32];
					api.GetTrackName(admVst->getTrackInstance().get(), buf, 32);
					msg.append(buf);
					msg.append("' has too few channels for ADM essence type specified by VST");
					warningStrings.push_back(msg);
				}
			}
		}

		if (knownSampleRate == 0) {
			knownSampleRate = vstSampleRate;
		}
		else if (knownSampleRate != vstSampleRate) {
			sampleRatesMatch = false;
		}
	}

	if (notIncluded > 0) {
		op = std::to_string(notIncluded);
		op.append((notIncluded == 1) ? " instance does not have 'Include in Render' set" : " instances do not have 'Include in Render' set");
		infoStrings.push_back(op);
	}

	if (offlineOrBypassed > 0) {
		op = std::to_string(offlineOrBypassed);
		op.append((offlineOrBypassed == 1) ? " instance is bypassed or offline" : " instances are bypassed or offline");
		infoStrings.push_back(op);
	}

	if (knownSampleRate == 0) errorStrings.push_back("Unable to determine sample rate from VSTs");
	if (!sampleRatesMatch) errorStrings.push_back("VSTs are reporting conflicting sample rates");

	for (auto& candidate : candidatesForExport) {
		for (auto& admAuthoringError : *(candidate->getAdmAuthoringErrors())) {
			warningStrings.push_back(admAuthoringError.what());
		}
	}


}

int AdmVstExportSources::getSampleRate()
{
	for (auto& thisVst : allAdmVsts) {
		int sr = thisVst->getSampleRate();
		if (sr > 0) return sr;
	}
	return 0;
}

int AdmVstExportSources::getTotalExportChannels()
{
	int numChns = 0;
	for (auto& candidate : candidatesForExport) {
		numChns += candidate->getPlugin()->getChannelCount();
	}
	return numChns;
}

void AdmVstExportSources::setRenderInProgress(bool state)
{
	for (auto& candidate : candidatesForExport) {
		candidate->setRenderInProgressState(true);
	}
}

bool AdmVstExportSources::isFrameAvailable()
{
	for (auto& candidate : candidatesForExport) {
		auto communicator = candidate->getCommunicator();
		if (!communicator || !communicator->nextFrameAvailable()) {
			return false;
		}
	}
	return true;
}

bool AdmVstExportSources::writeNextFrameTo(float* bufferWritePointer, bool skipFrameAvailableCheck)
{
	if (!skipFrameAvailableCheck && !isFrameAvailable()) return false;

	for (auto& candidate : candidatesForExport) {
		auto communicator = candidate->getCommunicator();
		if (communicator->getReportedChannelCount() > 0) {
			if (!communicator->copyNextFrame(bufferWritePointer, true)) {
				throw std::runtime_error("copyNextFrame failed");
			}
			bufferWritePointer += communicator->getReportedChannelCount();
		}
	}
	return true;
}

std::shared_ptr<bw64::AxmlChunk> AdmVstExportSources::getAxmlChunk()
{
	std::stringstream xmlStream;
	adm::writeXml(xmlStream, admDocument);
	xmlStream << "<!-- Produced using the EAR Production Suite (version ";
	xmlStream << (eps::versionInfoAvailable() ? eps::currentVersion() : "unknown");
	xmlStream << "), from ADM Export Source plugins -->\n";
	return std::make_shared<bw64::AxmlChunk>(bw64::AxmlChunk(xmlStream.str()));
}

std::shared_ptr<bw64::ChnaChunk> AdmVstExportSources::getChnaChunk()
{
	using namespace adm;
	std::vector<bw64::AudioId> audioIds;

	// audioIds for CHNA chunk (must be done AFTER reassigning ADM IDs)
	int trackNumCounter = 1; // Tracks are 1-indexed!!!
	for (auto& metadata : candidatesForExport) {
		if (auto audioPackFormat = metadata->getAudioPackFormat()) {
			auto audioPackFormatIdStr = formatId(audioPackFormat->get<AudioPackFormatId>());
			for (auto& admTrack : *metadata->getAdmSubgraphs()) {
				if (admTrack->audioTrackFormat) {
					// BS.2076-1 style structure (ATUID->ATF->ASF->ACF)
					audioIds.push_back(bw64::AudioId(trackNumCounter++,
						formatId(admTrack->audioTrackUid->get<AudioTrackUidId>()),
						formatId(admTrack->audioTrackFormat->get<AudioTrackFormatId>()),
						audioPackFormatIdStr
					));
				}
				else {
					// BS.2076-2 style structure (ATUID->ACF)
					std::string cfId = formatId(admTrack->audioChannelFormat->get<AudioChannelFormatId>());
					cfId += "_00";
					audioIds.push_back(bw64::AudioId(trackNumCounter++,
						formatId(admTrack->audioTrackUid->get<AudioTrackUidId>()),
						cfId,
						audioPackFormatIdStr
					));
				}
			}
		}
	}

	return std::make_shared<bw64::ChnaChunk>(bw64::ChnaChunk(audioIds));
}




AdmVstExporter::AdmVstExporter(std::shared_ptr<AdmVst> admExportVst, std::shared_ptr<adm::AudioContent> parentContent, std::shared_ptr<adm::AudioObject> audioObject, ReaperAPI const& api) : admExportVst{ admExportVst }, audioObject{ audioObject }, parentContent{ parentContent }, parentDocument{ parentContent->getParent().lock() }
{
	assert(parentDocument);
	assignAdmMetadata(api);
}

AdmVstExporter::~AdmVstExporter()
{
}

bool AdmVstExporter::isCandidateForExport()
{
	// Does NOT check for correct spat plugins or spat plugin location at this stage!
	// - this is entirely an ADM Export Source VST check.
	// Spat plugin checks come during ADM metadata generation (assignAdmMetadata onwards)
	return AdmVst::isCandidateForExport(admExportVst);
}

void AdmVstExporter::assignAdmMetadata(ReaperAPI const& api)
{
	admAuthoringErrors.clear();

	if (admExportVst->getAdmTypeDefinition() == adm::TypeDefinition::UNDEFINED.get()) {
		// We can't include this in the render, because the type was Undefined in the ADM VST and we rely on the packformat to determine how many channels to write out
		std::string errorMessage{ "The " };
		errorMessage += *AdmVst::getVstNameStr();
		errorMessage += " plugin on track '";
		errorMessage += admExportVst->getTrackInstance().getName();
		errorMessage += "' is set to type 'Undefined'.";
		admAuthoringErrors.push_back(AdmAuthoringError(errorMessage));
		return;
	}

	if (admExportVst->isUsingPresetDefinition()) {
		// Probably DS/HOA
		newAdmPresetDefinitionReference(api);
	}
	else {
		// Probably Obj
		// We can still generate ADM - just no blocks - default initial block will be created
		newAdmSubgraph(api);
	}
}

std::shared_ptr<AdmSubgraphElements> AdmVstExporter::newAdmSubgraph(ReaperAPI const& api)
{
	auto suffix = std::to_string(admSubgraphs.size());
	suffix.insert(0, "_");

	auto objectName = audioObject->get<adm::AudioObjectName>().get();
	auto objectType = admExportVst->getAdmType();

	if (!audioPackFormat) {
		// create packformat for this audioObject
		audioPackFormat = adm::AudioPackFormat::create(adm::AudioPackFormatName(objectName), admExportVst->getAdmType());
		audioObject->addReference(audioPackFormat);
	}

	auto subgraph = std::make_shared<AdmSubgraphElements>();
	admSubgraphs.push_back(subgraph);

	subgraph->audioChannelFormat = adm::AudioChannelFormat::create(adm::AudioChannelFormatName(objectName + suffix), objectType);
	subgraph->audioTrackUid = adm::AudioTrackUid::create();
	// References
	audioPackFormat->addReference(subgraph->audioChannelFormat);
	audioObject->addReference(subgraph->audioTrackUid);
	subgraph->audioTrackUid->setReference(subgraph->audioChannelFormat);
	subgraph->audioTrackUid->setReference(audioPackFormat);

	createAndAddAudioBlocks(objectType, subgraph, api);

	return subgraph;
}

void AdmVstExporter::newAdmPresetDefinitionReference(ReaperAPI const& api)
{
	auto typeDefinition = admExportVst->getAdmType();
	int typeDefinitionValue = typeDefinition.get();
	int packFormatIdValue = admExportVst->getAdmPackFormat();
	int channelFormatIdValue = admExportVst->getAdmChannelFormat();
	assert(packFormatIdValue != ADM_VST_PACKFORMAT_UNSET_ID);

	auto& presets = AdmPresetDefinitionsHelper::getSingleton();
	auto pfData = presets.getPackFormatData(typeDefinitionValue, packFormatIdValue);

	if (pfData) {
		std::optional<int> specificCfIdValue;
		if (channelFormatIdValue != ADM_VST_CHANNELFORMAT_ALLCHANNELS_ID)
			specificCfIdValue = channelFormatIdValue;

		auto holder = presets.setupPresetDefinitionObject(parentDocument, audioObject,
			pfData->packFormat->get<adm::AudioPackFormatId>(), specificCfIdValue);

		audioPackFormat = holder.audioPackFormat;

		for (auto channel : holder.channels) {
			auto subgraph = std::make_shared<AdmSubgraphElements>();
			subgraph->audioChannelFormat = channel.audioChannelFormat;
			subgraph->audioTrackUid = channel.audioTrackUid;

			admSubgraphs.push_back(subgraph);
		}
	}
}

void AdmVstExporter::createAndAddAudioBlocks(adm::TypeDescriptor typeDescriptor, std::shared_ptr<AdmSubgraphElements> subgraph, ReaperAPI const& api)
{
		// Only needs doing for Object type - other types use presets (common defs)
		if (typeDescriptor != adm::TypeDefinition::OBJECTS) return;

    auto start = std::chrono::nanoseconds::zero();
    auto duration = toNs(api.GetProjectLength(nullptr));

    if(audioObject->has<adm::Start>()) {
        start = audioObject->get<adm::Start>().get().asNanoseconds();
        duration -= start;
    }
    if(audioObject->has<adm::Duration>()) {
        duration = audioObject->get<adm::Duration>().get().asNanoseconds();
    }

    auto cumulatedPointData = CumulatedPointData(start, start + duration);
		// Without a plugin instance, we'll just generate default block
    addBlockFormatsToChannelFormat(cumulatedPointData.generateAudioBlockFormatObjects(nullptr, nullptr, api), subgraph);

}

template<typename AudioBlockFormatType>
void AdmVstExporter::addBlockFormatsToChannelFormat(std::optional<std::vector<std::shared_ptr<AudioBlockFormatType>>> blocks, std::shared_ptr<AdmSubgraphElements> subgraph) {
	assert(blocks.has_value());
	blockCount = (*blocks).size();
	for (auto& block : *blocks) {
		subgraph->audioChannelFormat->add(*block);
	}
}



AdmVstCommunicator* AdmVstExporter::getCommunicator(bool mustExist) {
	if (mustExist && !isCommunicatorPresent()) obtainCommunicator();
	return communicator.get();
}

bool AdmVstExporter::isCommunicatorPresent() {
	return (bool)communicator;
}

void AdmVstExporter::obtainCommunicator() {
	auto samplesPort = admExportVst->getSamplesSocketPort();
	auto commandPort = admExportVst->getCommandSocketPort();
	if (isCommunicatorPresent()) releaseCommunicator();
	communicator = CommunicatorRegistry::getCommunicator<AdmVstCommunicator>(samplesPort, commandPort);
}

void AdmVstExporter::releaseCommunicator() {
	communicator.reset();
}

bool AdmVstExporter::getRenderInProgressState() {
	if (!isCommunicatorPresent()) return false;
	return communicator->getRenderingState();
}

void AdmVstExporter::setRenderInProgressState(bool state) {
	if (state == true && !isCommunicatorPresent()) obtainCommunicator();
	if (isCommunicatorPresent()) communicator->setRenderingState(state);
}

AdmVstCommunicator::AdmVstCommunicator(int samplesPort, int commandPort) : CommunicatorBase(samplesPort, commandPort)
{
	updateInfo();
}

void AdmVstCommunicator::updateInfo()
{
	infoExchange();
}

void AdmVstCommunicator::infoExchange()
{
	auto resp = commandSocket.doCommand(commandSocket.Command::GetConfig);
	assert(resp->success());

	memcpy(&channelCount, (char*)resp->getBufferPointer() + 0, 1);
	memcpy(&sampleRate, (char*)resp->getBufferPointer() + 1, 4);
	memcpy(&admTypeDefinition, (char*)resp->getBufferPointer() + 5, 2);
	memcpy(&admPackFormatId, (char*)resp->getBufferPointer() + 7, 2);
	memcpy(&admChannelFormatId, (char*)resp->getBufferPointer() + 9, 2);

	infoReceived = true;
}