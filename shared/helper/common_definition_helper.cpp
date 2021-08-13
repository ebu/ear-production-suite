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
    if(!admCommonDefinitions) {
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
    if(it == typeDefinitionDatas.end()) return nullptr;
    return it->second;
}

std::shared_ptr<AdmCommonDefinitionHelper::PackFormatData> AdmCommonDefinitionHelper::getPackFormatData(int tdId, int pfId)
{
    auto td = getTypeDefinitionData(tdId);
    if(!td) return nullptr;
    for(auto pf : td->relatedPackFormats) {
        if(pf->id == pfId) return pf;
    }
    return nullptr;
}

std::shared_ptr<AdmCommonDefinitionHelper::ChannelFormatData> AdmCommonDefinitionHelper::getChannelFormatData(int tdId, int pfId, int cfId)
{
    auto pf = getPackFormatData(tdId, pfId);
    if(!pf) return nullptr;
    for(auto cf : pf->relatedChannelFormats) {
        if(cf->id == cfId) return cf;
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
    for(auto pf : packFormats) {
        int thisTypeDefinition = pf->get<adm::TypeDescriptor>().get();
        if(thisTypeDefinition == tdData->id) {
            auto pfId = pf->get<adm::AudioPackFormatId>().get<adm::AudioPackFormatIdValue>().get();

            auto pfData = std::make_shared<PackFormatData>();
            pfData->id = pfId;
            pfData->name = pf->get<adm::AudioPackFormatName>().get();
            pfData->niceName = makeNicePackFormatName(pfData->name);
            pfData->packFormat = pf;

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
    for(auto subPackFormat : subPackFormats) {
        recursePackFormatsForChannelFormats(subPackFormat, forPackFormatData);
    }

    // Find Channel formats
    auto channelFormats = fromPackFormat->getReferences<adm::AudioChannelFormat>();
    for(auto cf : channelFormats) {

        auto cfData = std::make_shared<ChannelFormatData>();
        cfData->id = cf->get<adm::AudioChannelFormatId>().get<adm::AudioChannelFormatIdValue>().get();
        cfData->name = cf->get<adm::AudioChannelFormatName>().get();
        cfData->channelFormat = cf;
        cfData->immediatePackFormat = fromPackFormat;

        forPackFormatData->relatedChannelFormats.push_back(cfData);
    }
}

std::string AdmCommonDefinitionHelper::makeNicePackFormatName(std::string originalName)
{
    std::string newName = originalName;

    // Tidy up name
    /// Extract ITU standard name if present
    auto standard = std::string("");
    if(newName.rfind("urn:itu:bs:", 0) == 0 && std::count(newName.begin(), newName.end(), ':') > 4) {
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
    if(colonPos != std::string::npos) {
        newName = newName.substr(colonPos + 1);
    }
    std::replace(newName.begin(), newName.end(), '_', ' ');
    assert(newName.length() > 0);
    newName[0] = toupper(newName[0]);
    ///Apply standard (if present)
    newName += standard;

    return newName;
}
