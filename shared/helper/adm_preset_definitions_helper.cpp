#include "adm_preset_definitions_helper.h"
#include <adm/common_definitions.hpp>
#include <adm/parse.hpp>
#include <algorithm>
#include <string_view>
#include <sstream>
#include <cassert>
#include <helper/container_helpers.hpp>
#include <helper/supplementary_definitions.hpp>
#include <helper/cartesianspeakerlayouts.h>
#include <helper/adm_equality_helpers.h>

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

	bool getItuUrnDetails(const std::string& possItuUrn, const std::string& onlyForLabelType,
		std::optional<std::string>& outItuStandard, std::optional<std::string>& outItuLabel)
	{
		if (possItuUrn.compare(0, 11, "urn:itu:bs:") == 0) {
			auto sections = splitString(possItuUrn, ':', 7);
			assert(sections.size() >= 7);
			// we already know the first 3 elms are "urn", "itu", "bs"
			if (sections.size() >= 7 && sections[5] == onlyForLabelType) {
				outItuStandard = std::string("BS.").append(sections[3]).append("-").append(sections[4]);
				outItuLabel = std::string(sections[6]);
				return true;
			}
		}
		return false;
	}
}

std::shared_ptr<AdmPresetDefinitionsHelper> AdmPresetDefinitionsHelper::getSingleton()
{
	static auto instance = std::make_shared<AdmPresetDefinitionsHelper>();
	return instance;
}

AdmPresetDefinitionsHelper::ObjectHolder AdmPresetDefinitionsHelper::addPresetDefinitionObjectTo(
	std::shared_ptr<adm::Document> document, const std::string& name, const adm::AudioPackFormatId packFormatId)
{
	auto audioObject = adm::AudioObject::create(adm::AudioObjectName(name));
	auto holder = setupPresetDefinitionObject(document, audioObject, packFormatId);
	document->add(holder.audioObject);
	return holder;
}

AdmPresetDefinitionsHelper::ObjectHolder AdmPresetDefinitionsHelper::setupPresetDefinitionObject(
	std::shared_ptr<adm::Document> document, std::shared_ptr<adm::AudioObject> existingAudioObject, const adm::AudioPackFormatId packFormatId, std::optional<int> forSingleCfIdValue)
{
	ObjectHolder holder;
	holder.audioObject = existingAudioObject;

	auto td = packFormatId.get<adm::TypeDescriptor>().get();
	auto pfIdVal = packFormatId.get<adm::AudioPackFormatIdValue>().get();
	auto pfData = getPackFormatData(td, pfIdVal);
	holder.audioPackFormat = document->lookup(packFormatId);

	if (!holder.audioPackFormat) {
		if (!pfData) {
			// not a preset (or therefore a common def)
			std::stringstream ss;
			ss << "AudioPackFormatId \"" << adm::formatId(packFormatId)
				<< "\" not found in Preset Definitions. Can't author ADM.";
			throw adm::error::AdmException(ss.str());
		}

		if (pfData->isCommonDefinition()) {
			// common def but not already in doc
			adm::addCommonDefinitionsTo(document);
		}

		// It's a supplementary definition - add the necessary PF/CF tree
		holder.audioPackFormat = copyMissingTreeElms(pfData->packFormat, document);
	}

	holder.audioObject->addReference(holder.audioPackFormat);
	if (pfData) {
		for (auto const& cfData : pfData->relatedChannelFormats) {
			if (!forSingleCfIdValue || *forSingleCfIdValue == cfData->idValue) {
				auto cfId = cfData->channelFormat->get<adm::AudioChannelFormatId>();
				auto cf = document->lookup(cfId);
				if (!cf) {
					std::stringstream ss;
					ss << "AudioChannelFormatId \"" << adm::formatId(cfId)
						<< "\" not found in document. Id might be invalid.";
					throw adm::error::AdmException(ss.str());
				}
				auto uid = adm::AudioTrackUid::create();
				uid->setReference(holder.audioPackFormat);
				uid->setReference(cf);
				holder.audioObject->addReference(uid);
				holder.channels.push_back(ChannelTrackAssociation{ cf, uid });
			}
		}
	}
	document->add(holder.audioObject);
	return holder;
}

std::vector<std::shared_ptr<AdmPresetDefinitionsHelper::TypeDefinitionData const>> AdmPresetDefinitionsHelper::getElementRelationships()
{
	if (!presetDefinitions) {
		//presetDefinitions = adm::getCommonDefinitions();
		std::istringstream xmlIs { xmlSupplementaryDefinitions };
		presetDefinitions = adm::parseXml(xmlIs, adm::xml::ParserOptions::recursive_node_search); // will also add common defs
		populateElementRelationshipsFor(adm::TypeDefinition::UNDEFINED);
		populateElementRelationshipsFor(adm::TypeDefinition::OBJECTS);
		populateElementRelationshipsFor(adm::TypeDefinition::MATRIX);
		populateElementRelationshipsFor(adm::TypeDefinition::DIRECT_SPEAKERS);
		populateElementRelationshipsFor(adm::TypeDefinition::HOA);
		populateElementRelationshipsFor(adm::TypeDefinition::BINAURAL);
	}
	return typeDefinitionDatas;
}

std::shared_ptr<AdmPresetDefinitionsHelper::TypeDefinitionData const> AdmPresetDefinitionsHelper::getTypeDefinitionData(int tdId)
{
	getElementRelationships(); // Ensures populated
	if (tdId < 0 || tdId >= typeDefinitionDatas.size()) {
		return nullptr;
	}
	return typeDefinitionDatas[tdId];
}

std::shared_ptr<AdmPresetDefinitionsHelper::PackFormatData const> AdmPresetDefinitionsHelper::getPackFormatData(int tdId, int pfId)
{
	auto td = getTypeDefinitionData(tdId);
	if (!td) return nullptr;
	for (auto pf : td->relatedPackFormats) {
		if (pf->idValue == pfId) return pf;
	}
	return nullptr;
}

std::shared_ptr<AdmPresetDefinitionsHelper::ChannelFormatData const> AdmPresetDefinitionsHelper::getChannelFormatData(int tdId, int pfId, int cfId)
{
	auto pf = getPackFormatData(tdId, pfId);
	if (!pf) return nullptr;
	for (auto cf : pf->relatedChannelFormats) {
		if (cf->idValue == cfId) return cf;
	}
	return nullptr;
}

std::shared_ptr<AdmPresetDefinitionsHelper::PackFormatData const> AdmPresetDefinitionsHelper::getPackFormatData(std::shared_ptr<const adm::AudioPackFormat> packFormat)
{
	if (!packFormat) return nullptr;

	auto td = packFormat->get<adm::TypeDescriptor>().get();
	auto packFormatIdValue = packFormat->get<adm::AudioPackFormatId>().get<adm::AudioPackFormatIdValue>().get();
	auto pfData = getPackFormatData(td, packFormatIdValue);

	// Common defs should always be matched by ID
	if (isCommonDefinition(packFormatIdValue))
		return pfData;

	// Not common def - Do more thorough checks

	/// Could be cart pf
	auto cartLayout = admplug::getCartLayout(*packFormat);
	if (cartLayout) {
		auto altPfIdStr = getMappedCommonPackId(*cartLayout);
		auto altPfId = adm::parseAudioPackFormatId(altPfIdStr);
		auto altPfIdValue = altPfId.get<adm::AudioPackFormatIdValue>().get();
		pfData = getPackFormatData(td, altPfIdValue);
		if (pfData) return pfData;
	}

	/// Some third-party tools do not use consistent PF IDs for their custom definitions. Try lookup by channels.
	pfData = getPackFormatDataByMatchingChannels(packFormat);
	if (pfData) return pfData;

	return nullptr;
}

std::shared_ptr<AdmPresetDefinitionsHelper::PackFormatData const> AdmPresetDefinitionsHelper::getPackFormatDataByMatchingChannels(std::shared_ptr<const adm::AudioPackFormat> packFormat)
{
	if(!packFormat) return nullptr;

	std::vector<std::pair<unsigned int, std::shared_ptr<const adm::AudioChannelFormat>>> cfsToFind;
	auto cfs = packFormat->getReferences<adm::AudioChannelFormat>();
	for (auto cf : cfs) {
		auto cfIdValue = cf->get<adm::AudioChannelFormatId>().get<adm::AudioChannelFormatIdValue>().get();
		cfsToFind.emplace_back(cfIdValue, std::move(cf));
	}

	auto td = packFormat->get<adm::TypeDescriptor>().get();
	for (auto pfData : getTypeDefinitionData(td)->relatedPackFormats) {
		auto pfDataCfDatas = pfData->relatedChannelFormats;
		if (pfDataCfDatas.size() == cfsToFind.size()) {
			bool matching = true;
			// Ordering must also be exact
			for (int i = 0; i < pfDataCfDatas.size(); ++i) {
				// Common Defs must always be matched by ID, because they're consistent
				if (isCommonDefinition(cfsToFind[i].first)) {
					if (pfDataCfDatas[i]->idValue != cfsToFind[i].first) {
						matching = false;
						break;
					}
				}
				// Custom CF's must be matched by properties, because they won't have consistent ID's
				else {
					if (!adm::helpers::equality::isEquivalentByProperties(pfDataCfDatas[i]->channelFormat, cfsToFind[i].second)) {
						matching = false;
						break;
					}
				}
			}
			if (matching) {
				return pfData;
			}
		}
	}

	return std::shared_ptr<PackFormatData>();
}

bool AdmPresetDefinitionsHelper::isCommonDefinition(int idValue)
{
	return idValue <= 0x0FFF;
}

std::shared_ptr<adm::AudioPackFormat> AdmPresetDefinitionsHelper::copyMissingTreeElms(std::shared_ptr<adm::AudioPackFormat> srcPf, std::shared_ptr<adm::Document> dstDoc)
{
	auto pfCopy = srcPf->copy();
	dstDoc->add(pfCopy);
	for (auto subPf : srcPf->getReferences<adm::AudioPackFormat>()) {
		auto subPfId = subPf->get<adm::AudioPackFormatId>();
		auto docSubPf = dstDoc->lookup(subPfId);
		if (!docSubPf) {
			docSubPf = copyMissingTreeElms(subPf, dstDoc);
		}
		pfCopy->addReference(docSubPf);
	}
	for (auto cf : srcPf->getReferences<adm::AudioChannelFormat>()) {
		auto cfId = cf->get<adm::AudioChannelFormatId>();
		auto docCf = dstDoc->lookup(cfId);
		if (!docCf) {
			auto cfCopy = cf->copy();
			dstDoc->add(cfCopy);
			docCf = cfCopy;
		}
		pfCopy->addReference(docCf);
	}
	return pfCopy;	
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

			tdData->relatedPackFormats.emplace_back(std::move(pfData));
		}
	}
	if (tdData->id >= typeDefinitionDatas.size()) {
		typeDefinitionDatas.resize(tdData->id + 1, nullptr);
	}
	typeDefinitionDatas[tdData->id] = std::move(tdData);
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



AdmPresetDefinitionsHelper::ChannelFormatData::ChannelFormatData(std::shared_ptr<adm::AudioChannelFormat> cf, std::shared_ptr<adm::AudioPackFormat> fromPackFormat) :
	idValue{ static_cast<int>(cf->get<adm::AudioChannelFormatId>().get<adm::AudioChannelFormatIdValue>().get()) },
	fullId{ adm::formatId(cf->get<adm::AudioChannelFormatId>()) },
	name{ cf->get<adm::AudioChannelFormatName>().get() },
	channelFormat{ cf },
	immediatePackFormat{ fromPackFormat }
{
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

const bool AdmPresetDefinitionsHelper::ChannelFormatData::isCommonDefinition() const
{
	return AdmPresetDefinitionsHelper::isCommonDefinition(idValue);
}

const std::string AdmPresetDefinitionsHelper::ChannelFormatData::getBestSpeakerLabel() const
{
	if (legacySpeakerLabel)
		return *legacySpeakerLabel;
	if (ituLabel)
		return *ituLabel;
	if (speakerLabels.size() > 0)
		return speakerLabels[0];
	return name;
}

void AdmPresetDefinitionsHelper::ChannelFormatData::setItuLabels()
{
	const std::string ituLabelType{ "speaker" };
	for (auto const& speakerLabel : speakerLabels) {
		if (getItuUrnDetails(speakerLabel, ituLabelType, ituStandard, ituLabel)) {
			break;
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

AdmPresetDefinitionsHelper::PackFormatData::PackFormatData(std::shared_ptr<adm::AudioPackFormat> pf) :
	idValue{ static_cast<int>(pf->get<adm::AudioPackFormatId>().get<adm::AudioPackFormatIdValue>().get()) },
	fullId{ adm::formatId(pf->get<adm::AudioPackFormatId>()) },
	name{ pf->get<adm::AudioPackFormatName>().get() },
	packFormat{ pf }
{
	setLabels();
}

AdmPresetDefinitionsHelper::PackFormatData::~PackFormatData()
{
}

const bool AdmPresetDefinitionsHelper::PackFormatData::isCommonDefinition() const
{
	return AdmPresetDefinitionsHelper::isCommonDefinition(idValue);
}

void AdmPresetDefinitionsHelper::PackFormatData::setLabels()
{
	const std::string ituLabelType{ "pack" };
	getItuUrnDetails(name, ituLabelType, ituStandard, ituLabel);
	niceName = ituLabel ? *ituLabel : name;
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