#pragma once

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <optional>
#include <vector>
#include <map>
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
		std::optional<std::string> legacySpeakerLabel;
		std::optional<std::string> ituLabel;
		std::optional<std::string> ituStandard;
		std::shared_ptr<adm::AudioChannelFormat> channelFormat;
		std::shared_ptr<adm::AudioPackFormat> immediatePackFormat;
		bool isLfe{ false };
		std::vector<std::string> speakerLabels;
		float azimuth{ 0.f };
		float elevation{ 0.f };
		float distance{ 1.f	};

	private:
		void setItuLabels();
		void setLegacySpeakerLabel();

		struct ChannelFormatNiceName {
			const std::string defaultLegacySpeakerLabel;
			const std::map<const std::string, const std::string> specificForPackFormatId = {};
		};
		static const std::map<const std::string, const ChannelFormatNiceName> channelFormatNiceNames;
	};

	class PackFormatData {
	public:
		PackFormatData(std::shared_ptr<adm::AudioPackFormat> apf);
		~PackFormatData();

		int idValue{ 0 };
		std::string fullId;
		std::string name;
		std::string niceName;
		std::optional<std::string> ituLabel;
		std::optional<std::string> ituStandard;
		std::shared_ptr<adm::AudioPackFormat> packFormat;
		std::vector<std::shared_ptr<ChannelFormatData>> relatedChannelFormats;

	private:
		void setLabels();
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
	std::shared_ptr<adm::Document> admCommonDefinitions;
	std::map<int, std::shared_ptr<TypeDefinitionData>> typeDefinitionDatas;
};