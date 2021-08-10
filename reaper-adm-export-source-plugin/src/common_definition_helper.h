#pragma once

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <adm/adm.hpp>

class AdmCommonDefinitionHelper{
public:
    AdmCommonDefinitionHelper();
    ~AdmCommonDefinitionHelper();

    static std::shared_ptr<AdmCommonDefinitionHelper> getSingleton();

    struct ChannelFormatData {
        int id;
        std::string name;
        std::shared_ptr<adm::AudioChannelFormat> channelFormat;
        std::shared_ptr<adm::AudioPackFormat> immediatePackFormat;
    };

    struct PackFormatData {
        int id;
        std::string name;
        std::string niceName;
        std::shared_ptr<adm::AudioPackFormat> packFormat;
        std::vector<std::shared_ptr<ChannelFormatData>> relatedChannelFormats;
    };

    struct TypeDefinitionData {
        int id;
        adm::TypeDescriptor typeDescriptor;
        std::string name;
        std::vector<std::shared_ptr<PackFormatData>> relatedPackFormats;
    };

    std::map<int, std::shared_ptr<TypeDefinitionData>> getElementRelationships();

    std::shared_ptr<TypeDefinitionData> getTypeDefinitionData(int tdId);
    std::shared_ptr<PackFormatData> getPackFormatData(int tdId, int pfId);
    std::shared_ptr<ChannelFormatData> getChannelFormatData(int tdId, int pfId, int cfId);

private:
    void populateElementRelationshipsFor(adm::TypeDescriptor);
    void recursePackFormatsForChannelFormats(std::shared_ptr<adm::AudioPackFormat> fromPackFormat, std::shared_ptr<PackFormatData> forPackFormatData);
    std::string makeNicePackFormatName(std::string originalName);
    std::shared_ptr<adm::Document> admCommonDefinitions;
    std::map<int, std::shared_ptr<TypeDefinitionData>> typeDefinitionDatas;
};