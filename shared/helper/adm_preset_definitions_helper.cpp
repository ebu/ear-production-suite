#include "adm_preset_definitions_helper.h"
#include <adm/common_definitions.hpp>
#include <algorithm>
#include <string>
#include <vector>
#include <string_view>
#include <helper/container_helpers.hpp>

namespace {
	std::vector<std::string_view> splitString(const std::string& input, char delimiter, size_t maxElements) {
		std::vector<std::string_view> result;
		std::string_view str(input);
		size_t start = 0;

		for (size_t i = 0; i < maxElements - 1; ++i) {
			size_t end = str.find(delimiter, start);
			if (end == std::string_view::npos) {
				break;
			}
			result.push_back(str.substr(start, end - start));
			start = end + 1;
		}

		// Add the remaining text, even if it contains more delimiters
		result.push_back(str.substr(start));

		return result;
	}
}

AdmPresetDefinitionsHelper::AdmPresetDefinitionsHelper()
{
}

AdmPresetDefinitionsHelper::~AdmPresetDefinitionsHelper()
{
}

std::shared_ptr<AdmPresetDefinitionsHelper> AdmPresetDefinitionsHelper::getSingleton()
{
	static auto instance = std::make_shared<AdmPresetDefinitionsHelper>();
	return instance;
}

std::map<int, std::shared_ptr<AdmPresetDefinitionsHelper::TypeDefinitionData>> AdmPresetDefinitionsHelper::getElementRelationships()
{
	if (!presetDefinitions) {
		presetDefinitions = adm::getCommonDefinitions();
		populateElementRelationshipsFor(adm::TypeDefinition::UNDEFINED);
		populateElementRelationshipsFor(adm::TypeDefinition::OBJECTS);
		populateElementRelationshipsFor(adm::TypeDefinition::DIRECT_SPEAKERS);
		populateElementRelationshipsFor(adm::TypeDefinition::HOA);
		populateElementRelationshipsFor(adm::TypeDefinition::BINAURAL);
	}
	return typeDefinitionDatas;
}

std::shared_ptr<AdmPresetDefinitionsHelper::TypeDefinitionData> AdmPresetDefinitionsHelper::getTypeDefinitionData(int tdId)
{
	getElementRelationships(); // Ensures populated
	auto it = typeDefinitionDatas.find(tdId);
	if (it == typeDefinitionDatas.end()) return nullptr;
	return it->second;
}

std::shared_ptr<AdmPresetDefinitionsHelper::PackFormatData> AdmPresetDefinitionsHelper::getPackFormatData(int tdId, int pfId)
{
	auto td = getTypeDefinitionData(tdId);
	if (!td) return nullptr;
	for (auto pf : td->relatedPackFormats) {
		if (pf->idValue == pfId) return pf;
	}
	return nullptr;
}

std::shared_ptr<AdmPresetDefinitionsHelper::ChannelFormatData> AdmPresetDefinitionsHelper::getChannelFormatData(int tdId, int pfId, int cfId)
{
	auto pf = getPackFormatData(tdId, pfId);
	if (!pf) return nullptr;
	for (auto cf : pf->relatedChannelFormats) {
		if (cf->idValue == cfId) return cf;
	}
	return nullptr;
}

void AdmPresetDefinitionsHelper::populateElementRelationshipsFor(adm::TypeDescriptor typeDescriptor)
{
	auto tdData = std::make_shared<TypeDefinitionData>();
	tdData->id = typeDescriptor.get();
	tdData->typeDescriptor = typeDescriptor;
	tdData->name = adm::formatTypeDefinition(typeDescriptor);

	auto packFormats = presetDefinitions->getElements<adm::AudioPackFormat>();
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

void AdmPresetDefinitionsHelper::recursePackFormatsForChannelFormats(std::shared_ptr<adm::AudioPackFormat> fromPackFormat, std::shared_ptr<PackFormatData> forPackFormatData)
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



AdmPresetDefinitionsHelper::ChannelFormatData::ChannelFormatData(std::shared_ptr<adm::AudioChannelFormat> cf, std::shared_ptr<adm::AudioPackFormat> fromPackFormat)
{
	idValue = cf->get<adm::AudioChannelFormatId>().get<adm::AudioChannelFormatIdValue>().get();
	fullId = adm::formatId(cf->get<adm::AudioChannelFormatId>());
	name = cf->get<adm::AudioChannelFormatName>().get();
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

		setItuLabels();
		setLegacySpeakerLabel();
	}
}

AdmPresetDefinitionsHelper::ChannelFormatData::~ChannelFormatData()
{
}

void AdmPresetDefinitionsHelper::ChannelFormatData::setItuLabels()
{
	for (auto const& speakerLabel : speakerLabels) {
		if (speakerLabel.compare(0, 11, "urn:itu:bs:") == 0){
			auto sections = splitString(speakerLabel, ':', 7);
			// we already know the first 3 elms are "urn", "itu", "bs"
			if (sections[5] == "speaker") {
				ituStandard = std::string("BS.").append(sections[3]).append("-").append(sections[4]);
				ituLabel = std::string(sections[6]);
				return;
			}
		}
	}
}

void AdmPresetDefinitionsHelper::ChannelFormatData::setLegacySpeakerLabel()
{
	auto itLabels = channelFormatNiceNames.find(fullId);
	if (itLabels == channelFormatNiceNames.end()){
		return;
	}
	auto const &labels = itLabels->second;
	legacySpeakerLabel = labels.defaultLegacySpeakerLabel;
	if (!labels.specificForPackFormatId.empty()) {
		auto pfId = adm::formatId(immediatePackFormat->get<adm::AudioPackFormatId>());
		auto itPfSpecificLabel = labels.specificForPackFormatId.find(pfId);
		if (itPfSpecificLabel != labels.specificForPackFormatId.end()) {
			legacySpeakerLabel = itPfSpecificLabel->second;
		}
	}
}

AdmPresetDefinitionsHelper::PackFormatData::PackFormatData(std::shared_ptr<adm::AudioPackFormat> pf)
{
	idValue = pf->get<adm::AudioPackFormatId>().get<adm::AudioPackFormatIdValue>().get();
	fullId = adm::formatId(pf->get<adm::AudioPackFormatId>());
	name = pf->get<adm::AudioPackFormatName>().get();
	packFormat = pf;
	setLabels();
}

AdmPresetDefinitionsHelper::PackFormatData::~PackFormatData()
{
}

void AdmPresetDefinitionsHelper::PackFormatData::setLabels()
{
	if (name.compare(0, 11, "urn:itu:bs:") == 0) {
		auto sections = splitString(name, ':', 7);
		// we already know the first 3 elms are "urn", "itu", "bs"
		if (sections[5] == "pack") {
			ituStandard = std::string("BS.").append(sections[3]).append("-").append(sections[4]);
			ituLabel = std::string(sections[6]);
		}
	}

	niceName = ituLabel.has_value() ? ituLabel.value() : name;
	std::replace(niceName.begin(), niceName.end(), '_', ' ');
	assert(niceName.length() > 0);
	niceName[0] = toupper(niceName[0]);
}


/* clang-format off */
const std::map<const std::string, const AdmPresetDefinitionsHelper::ChannelFormatData::ChannelFormatNiceName> 
	AdmPresetDefinitionsHelper::ChannelFormatData::channelFormatNiceNames = {
	{"AC_00010001", ChannelFormatNiceName{ "L", {
		{"AP_00010009", "FLc"},
	} }},
	{ "AC_00010002", ChannelFormatNiceName{ "R", {
		{"AP_00010009", "FRc"},
	}} },
	{ "AC_00010003", ChannelFormatNiceName{ "C", {
		{"AP_00010001", "M"},
		{"AP_00010009", "FC"},
	}} },
	{ "AC_00010004", ChannelFormatNiceName{ "LFE" } },
	{ "AC_00010005", ChannelFormatNiceName{ "Ls" } },
	{ "AC_00010006", ChannelFormatNiceName{ "Rs" } },
	{ "AC_00010009", ChannelFormatNiceName{ "BC" } },
	{ "AC_0001000a", ChannelFormatNiceName{ "Lss", {
		{"AP_00010009", "SiL"},
		{"AP_00010016", "SiL"},
	}} },
	{ "AC_0001000b", ChannelFormatNiceName{ "Rss", {
		{"AP_00010009", "SiR"},
		{"AP_00010016", "SiR"},
	}} },
	{ "AC_0001000c", ChannelFormatNiceName{ "TpC" } },
	{ "AC_0001000d", ChannelFormatNiceName{ "Ltf" } },
	{ "AC_0001000e", ChannelFormatNiceName{ "TpFC" } },
	{ "AC_0001000f", ChannelFormatNiceName{ "Rtf" } },
	{ "AC_00010010", ChannelFormatNiceName{ "Ltr" } },
	{ "AC_00010011", ChannelFormatNiceName{ "TpBC" } },
	{ "AC_00010012", ChannelFormatNiceName{ "Rtr" } },
	{ "AC_00010013", ChannelFormatNiceName{ "TpSiL" } },
	{ "AC_00010014", ChannelFormatNiceName{ "TpSiR" } },
	{ "AC_00010015", ChannelFormatNiceName{ "Cbf", {
		{"AP_00010009", "BtFC"},
	}} },
	{ "AC_00010016", ChannelFormatNiceName{ "BtFL" } },
	{ "AC_00010017", ChannelFormatNiceName{ "BtFR" } },
	{ "AC_00010018", ChannelFormatNiceName{ "FL" } },
	{ "AC_00010019", ChannelFormatNiceName{ "FR" } },
	{ "AC_0001001a", ChannelFormatNiceName{ "BL" } },
	{ "AC_0001001b", ChannelFormatNiceName{ "BR" } },
	{ "AC_0001001c", ChannelFormatNiceName{ "Lrs", {
		{"AP_00010007", "LB"},
		{"AP_00010009", "BL"},
		{"AP_00010016", "LB"},
	}} },
	{ "AC_0001001d", ChannelFormatNiceName{ "Rrs", {
		{"AP_00010007", "RB"},
		{"AP_00010009", "BR"},
		{"AP_00010016", "RB"},
	}} },
	{ "AC_0001001e", ChannelFormatNiceName{ "Ltb", {
		{"AP_00010009", "TpBL"},
	}} },
	{ "AC_0001001f", ChannelFormatNiceName{ "Rtb", {
		{"AP_00010009", "TpBR"},
	}} },
	{ "AC_00010020", ChannelFormatNiceName{ "LFE1" } },
	{ "AC_00010021", ChannelFormatNiceName{ "LFE2" } },
	{ "AC_00010022", ChannelFormatNiceName{ "LH", {
		{"AP_00010009", "TpFL"},
	}} },
	{ "AC_00010023", ChannelFormatNiceName{ "RH", {
		{"AP_00010009", "TpFR"},
	}} },
	{ "AC_00010024", ChannelFormatNiceName{ "Lsc" } },
	{ "AC_00010025", ChannelFormatNiceName{ "Rsc" } },
	{ "AC_00010026", ChannelFormatNiceName{ "FLM" } },
	{ "AC_00010027", ChannelFormatNiceName{ "FRM" } },
	{ "AC_00010028", ChannelFormatNiceName{ "CH" } },
};
/* clang-format on */