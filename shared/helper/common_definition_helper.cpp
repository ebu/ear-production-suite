#include "common_definition_helper.h"
#include <adm/common_definitions.hpp>
#include <algorithm>

AdmCommonDefinitionHelper::AdmCommonDefinitionHelper()
{
}

AdmCommonDefinitionHelper::~AdmCommonDefinitionHelper()
{
}

std::shared_ptr<AdmCommonDefinitionHelper> AdmCommonDefinitionHelper::getSingleton()
{
	static auto instance = std::make_shared<AdmCommonDefinitionHelper>();
	return instance;
}

std::map<int, std::shared_ptr<AdmCommonDefinitionHelper::TypeDefinitionData>> AdmCommonDefinitionHelper::getElementRelationships()
{
	if (!admCommonDefinitions) {
		admCommonDefinitions = adm::getCommonDefinitions();
		populateElementRelationshipsFor(adm::TypeDefinition::UNDEFINED);
		populateElementRelationshipsFor(adm::TypeDefinition::OBJECTS);
		populateElementRelationshipsFor(adm::TypeDefinition::DIRECT_SPEAKERS);
		populateElementRelationshipsFor(adm::TypeDefinition::HOA);
		populateElementRelationshipsFor(adm::TypeDefinition::BINAURAL);
	}
	return typeDefinitionDatas;
}

std::shared_ptr<AdmCommonDefinitionHelper::TypeDefinitionData> AdmCommonDefinitionHelper::getTypeDefinitionData(int tdId)
{
	getElementRelationships(); // Ensures populated
	auto it = typeDefinitionDatas.find(tdId);
	if (it == typeDefinitionDatas.end()) return nullptr;
	return it->second;
}

std::shared_ptr<AdmCommonDefinitionHelper::PackFormatData> AdmCommonDefinitionHelper::getPackFormatData(int tdId, int pfId)
{
	auto td = getTypeDefinitionData(tdId);
	if (!td) return nullptr;
	for (auto pf : td->relatedPackFormats) {
		if (pf->idValue == pfId) return pf;
	}
	return nullptr;
}

std::shared_ptr<AdmCommonDefinitionHelper::ChannelFormatData> AdmCommonDefinitionHelper::getChannelFormatData(int tdId, int pfId, int cfId)
{
	auto pf = getPackFormatData(tdId, pfId);
	if (!pf) return nullptr;
	for (auto cf : pf->relatedChannelFormats) {
		if (cf->idValue == cfId) return cf;
	}
	return nullptr;
}

void AdmCommonDefinitionHelper::populateElementRelationshipsFor(adm::TypeDescriptor typeDescriptor)
{
	auto tdData = std::make_shared<TypeDefinitionData>();
	tdData->id = typeDescriptor.get();
	tdData->typeDescriptor = typeDescriptor;
	tdData->name = adm::formatTypeDefinition(typeDescriptor);

	auto packFormats = admCommonDefinitions->getElements<adm::AudioPackFormat>();
	for (auto pf : packFormats) {
		int thisTypeDefinition = pf->get<adm::TypeDescriptor>().get();
		if (thisTypeDefinition == tdData->id) {

			auto pfData = std::make_shared<PackFormatData>(pf);
			// Find Channel formats
			recursePackFormatsForChannelFormats(pf, pfData);

			tdData->relatedPackFormats.push_back(pfData);
		}
	}
	typeDefinitionDatas.insert(std::make_pair(tdData->id, tdData));
}

void AdmCommonDefinitionHelper::recursePackFormatsForChannelFormats(std::shared_ptr<adm::AudioPackFormat> fromPackFormat, std::shared_ptr<PackFormatData> forPackFormatData)
{
	// Do other pack formats first (these channels will want to appear before the ones directly in this pack format)
	auto subPackFormats = fromPackFormat->getReferences<adm::AudioPackFormat>();
	for (auto subPackFormat : subPackFormats) {
		recursePackFormatsForChannelFormats(subPackFormat, forPackFormatData);
	}

	// Find Channel formats
	auto channelFormats = fromPackFormat->getReferences<adm::AudioChannelFormat>();
	for (auto cf : channelFormats) {
		auto cfData = std::make_shared<ChannelFormatData>(cf, fromPackFormat);
		forPackFormatData->relatedChannelFormats.push_back(cfData);
	}
}



AdmCommonDefinitionHelper::ChannelFormatData::ChannelFormatData(std::shared_ptr<adm::AudioChannelFormat> cf, std::shared_ptr<adm::AudioPackFormat> fromPackFormat)
{
	idValue = cf->get<adm::AudioChannelFormatId>().get<adm::AudioChannelFormatIdValue>().get();
	fullId = adm::formatId(cf->get<adm::AudioChannelFormatId>());
	name = cf->get<adm::AudioChannelFormatName>().get();
	niceName = name;
	channelFormat = cf;
	immediatePackFormat = fromPackFormat;

	if (cf->has<adm::Frequency>()) {
		auto freq = cf->get<adm::Frequency>();
		isLfe = adm::isLowPass(freq);
	}

	auto td = fromPackFormat->get<adm::TypeDescriptor>();
	if (td == adm::TypeDefinition::DIRECT_SPEAKERS) {
		auto bfs = cf->getElements<adm::AudioBlockFormatDirectSpeakers>();
		if (bfs.size() > 0) {
			auto const& bf = bfs[0];

			if (bf.has<adm::SphericalSpeakerPosition>()) {
				auto const& pos = bf.get<adm::SphericalSpeakerPosition>();
				if (pos.has<adm::Azimuth>()) {
					azimuth = pos.get<adm::Azimuth>().get();
				}
				if (pos.has<adm::Elevation>()) {
					elevation = pos.get<adm::Elevation>().get();
				}
				if (pos.has<adm::Distance>()) {
					distance = pos.get<adm::Distance>().get();
				}
			}

			auto bfSpeakerLabels = bf.get<adm::SpeakerLabels>();
			for (auto const& bfSpeakerLabel : bfSpeakerLabels) {
				speakerLabels.push_back(bfSpeakerLabel.get());
			}
		}
		if (speakerLabels.size() > 0) {
			niceName = makeNiceSpeakerName(speakerLabels);
		}
	}
}

AdmCommonDefinitionHelper::ChannelFormatData::~ChannelFormatData()
{
}

std::string AdmCommonDefinitionHelper::ChannelFormatData::makeNiceSpeakerName(const std::vector<std::string>& speakerLabels)
{
	// Use first speaker label
	if (speakerLabels.size() > 0) {
		std::string newName = speakerLabels[0];
		// Remove urn if present
		if (newName.rfind("urn:itu:bs:", 0) == 0 && std::count(newName.begin(), newName.end(), ':') > 4) {
			auto colonPos = newName.find_last_of(':');
			if (colonPos != std::string::npos) {
				newName = newName.substr(colonPos + 1);
			}
		}
		return newName;
	}
	return std::string();
}

AdmCommonDefinitionHelper::PackFormatData::PackFormatData(std::shared_ptr<adm::AudioPackFormat> pf)
{
	idValue = pf->get<adm::AudioPackFormatId>().get<adm::AudioPackFormatIdValue>().get();
	fullId = adm::formatId(pf->get<adm::AudioPackFormatId>());
	name = pf->get<adm::AudioPackFormatName>().get();
	niceName = makeNicePackFormatName(name);
	packFormat = pf;
}

AdmCommonDefinitionHelper::PackFormatData::~PackFormatData()
{
}

std::string AdmCommonDefinitionHelper::PackFormatData::makeNicePackFormatName(const std::string& originalName)
{
	std::string newName = originalName;

	// Tidy up name
	/// Extract ITU standard name if present
	auto standard = std::string("");
	if (newName.rfind("urn:itu:bs:", 0) == 0 && std::count(newName.begin(), newName.end(), ':') > 4) {
		auto standardStart = newName.find(':', 10);
		auto standardDiv = newName.find(':', standardStart + 1);
		auto standardEnd = newName.find(':', standardDiv + 1);
		auto standardNumber = newName.substr(standardStart + 1, standardDiv - standardStart - 1);
		auto standardRevision = newName.substr(standardDiv + 1, standardEnd - standardDiv - 1);
		standard += "  [BS. ";
		standard += standardNumber;
		standard += "-";
		standard += standardRevision;
		standard += "]";
	}
	///Tidy rest of string
	auto colonPos = newName.find_last_of(':');
	if (colonPos != std::string::npos) {
		newName = newName.substr(colonPos + 1);
	}
	std::replace(newName.begin(), newName.end(), '_', ' ');
	assert(newName.length() > 0);
	newName[0] = toupper(newName[0]);
	///Apply standard (if present)
	newName += standard;

	return newName;
}