#pragma once

#include <vector>
#include <map>
#include <memory>
#include <string>
#include <optional>
#include <vector>
#include <map>
#include <adm/adm.hpp>

class AdmPresetDefinitionsHelper {
public:
	AdmPresetDefinitionsHelper();
	~AdmPresetDefinitionsHelper();

	static std::shared_ptr<AdmPresetDefinitionsHelper> getSingleton();

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

		bool isCommonDefinition();
		std::string getBestSpeakerLabel();

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

		bool isCommonDefinition();

	private:
		void setLabels();
	};

	struct TypeDefinitionData {
		int id{ 0 };
		adm::TypeDescriptor typeDescriptor;
		std::string name;
		std::vector<std::shared_ptr<PackFormatData>> relatedPackFormats;
	};

	struct ChannelTrackAssociation {
		std::shared_ptr<adm::AudioChannelFormat> audioChannelFormat;
		std::shared_ptr<adm::AudioTrackUid> audioTrackUid;
	};

	struct ObjectHolder {
		std::shared_ptr<adm::AudioObject> audioObject;
		std::shared_ptr<adm::AudioPackFormat> audioPackFormat;
		std::vector<ChannelTrackAssociation> channels;
	};

	ObjectHolder addPresetDefinitionObjectTo(std::shared_ptr<adm::Document> document, 
		const std::string& name, 
		const adm::AudioPackFormatId packFormatId);
	ObjectHolder setupPresetDefinitionObject(std::shared_ptr<adm::Document> document, 
		std::shared_ptr<adm::AudioObject> existingAudioObject,
		const adm::AudioPackFormatId packFormatId,
		std::optional<int> forSingleCfIdValue = {});

	std::map<int, std::shared_ptr<TypeDefinitionData>> getElementRelationships();

	std::shared_ptr<TypeDefinitionData> getTypeDefinitionData(int tdId);
	std::shared_ptr<PackFormatData> getPackFormatData(int tdId, int pfId);
	std::shared_ptr<ChannelFormatData> getChannelFormatData(int tdId, int pfId, int cfId);

	std::shared_ptr<PackFormatData> getPackFormatData(std::shared_ptr<const adm::AudioPackFormat> packFormat);
	std::shared_ptr<PackFormatData> getPackFormatDataByMatchingChannels(std::shared_ptr<const adm::AudioPackFormat> packFormat);

	static bool isCommonDefinition(int idValue);
	static std::shared_ptr<adm::AudioPackFormat> copyMissingTreeElms(std::shared_ptr<adm::AudioPackFormat> srcPf, std::shared_ptr<adm::Document> dstDoc);

private:
	void populateElementRelationshipsFor(adm::TypeDescriptor);
	void recursePackFormatsForChannelFormats(std::shared_ptr<adm::AudioPackFormat> fromPackFormat, std::shared_ptr<PackFormatData> forPackFormatData);
	bool isEquivalentByProperties(std::shared_ptr<const adm::AudioChannelFormat> cfA, std::shared_ptr<const adm::AudioChannelFormat> cfB);
	std::shared_ptr<adm::Document> presetDefinitions;
	std::map<int, std::shared_ptr<TypeDefinitionData>> typeDefinitionDatas;
};