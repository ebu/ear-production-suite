#pragma once

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <adm/adm.hpp>

class AdmCommonDefinitionHelper {
public:
	AdmCommonDefinitionHelper();
	~AdmCommonDefinitionHelper();

	static std::shared_ptr<AdmCommonDefinitionHelper> getSingleton();

	class ChannelFormatData {
	public:
		ChannelFormatData(std::shared_ptr<adm::AudioChannelFormat> acf, std::shared_ptr<adm::AudioPackFormat> fromPackFormat);
		~ChannelFormatData();

		int idValue{ 0 };
		std::string fullId;
		std::string name;
		std::string niceName;
		std::shared_ptr<adm::AudioChannelFormat> channelFormat;
		std::shared_ptr<adm::AudioPackFormat> immediatePackFormat;
		bool isLfe{ false };
		std::vector<std::string> speakerLabels;
		float azimuth{ 0.f };
		float elevation{ 0.f };
		float distance{ 1.f	};

	private:
		std::string makeNiceSpeakerName(const std::vector<std::string>& speakerLabels);

	};

	struct PackFormatData {
		int idValue{ 0 };
		std::string fullId;
		std::string name;
		std::string niceName;
		std::shared_ptr<adm::AudioPackFormat> packFormat;
		std::vector<std::shared_ptr<ChannelFormatData>> relatedChannelFormats;
	};

	struct TypeDefinitionData {
		int id{ 0 };
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
	std::string makeNicePackFormatName(const std::string& originalName);
	std::shared_ptr<adm::Document> admCommonDefinitions;
	std::map<int, std::shared_ptr<TypeDefinitionData>> typeDefinitionDatas;
};