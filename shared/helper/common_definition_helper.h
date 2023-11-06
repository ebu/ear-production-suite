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

		/* clang-format off */
		std::map<const std::string, const ChannelFormatNiceName> channelFormatNiceNames{
			{"AC_00010001", ChannelFormatNiceName{ "L", {
				{"AP_00010009", "FLc"},
			}}},
			{"AC_00010002", ChannelFormatNiceName{ "R", {
				{"AP_00010009", "FRc"},
			}}},
			{"AC_00010003", ChannelFormatNiceName{ "C", {
				{"AP_00010001", "M"},
				{"AP_00010009", "FC"},
			}}},
			{"AC_00010004", ChannelFormatNiceName{ "LFE" }},
			{"AC_00010005", ChannelFormatNiceName{ "Ls" }},
			{"AC_00010006", ChannelFormatNiceName{ "Rs" }},
			{"AC_00010009", ChannelFormatNiceName{ "BC" }},
			{"AC_0001000a", ChannelFormatNiceName{ "Lss", {
				{"AP_00010009", "SiL"},
				{"AP_00010016", "SiL"},
			}}},
			{"AC_0001000b", ChannelFormatNiceName{ "Rss", {
				{"AP_00010009", "SiR"},
				{"AP_00010016", "SiR"},
			}}},
			{"AC_0001000c", ChannelFormatNiceName{ "TpC" }},
			{"AC_0001000d", ChannelFormatNiceName{ "Ltf" }},
			{"AC_0001000e", ChannelFormatNiceName{ "TpFC" }},
			{"AC_0001000f", ChannelFormatNiceName{ "Rtf" }},
			{"AC_00010010", ChannelFormatNiceName{ "Ltr" }},
			{"AC_00010011", ChannelFormatNiceName{ "TpBC" }},
			{"AC_00010012", ChannelFormatNiceName{ "Rtr" }},
			{"AC_00010013", ChannelFormatNiceName{ "TpSiL" }},
			{"AC_00010014", ChannelFormatNiceName{ "TpSiR" }},
			{"AC_00010015", ChannelFormatNiceName{ "Cbf", {
				{"AP_00010009", "BtFC"},
			}}},
			{"AC_00010016", ChannelFormatNiceName{ "BtFL" }},
			{"AC_00010017", ChannelFormatNiceName{ "BtFR" }},
			{"AC_00010018", ChannelFormatNiceName{ "FL" }},
			{"AC_00010019", ChannelFormatNiceName{ "FR" }},
			{"AC_0001001a", ChannelFormatNiceName{ "BL" }},
			{"AC_0001001b", ChannelFormatNiceName{ "BR" }},
			{"AC_0001001c", ChannelFormatNiceName{ "Lrs", {
				{"AP_00010007", "LB"},
				{"AP_00010009", "BL"},
				{"AP_00010016", "LB"},
			}}},
			{"AC_0001001d", ChannelFormatNiceName{ "Rrs", {
				{"AP_00010007", "RB"},
				{"AP_00010009", "BR"},
				{"AP_00010016", "RB"},
			}}},
			{"AC_0001001e", ChannelFormatNiceName{ "Ltb", {
				{"AP_00010009", "TpBL"},
			}}},
			{"AC_0001001f", ChannelFormatNiceName{ "Rtb", {
				{"AP_00010009", "TpBR"},
			}}},
			{"AC_00010020", ChannelFormatNiceName{ "LFE1" }},
			{"AC_00010021", ChannelFormatNiceName{ "LFE2" }},
			{"AC_00010022", ChannelFormatNiceName{ "LH", {
				{"AP_00010009", "TpFL"},
			}}},
			{"AC_00010023", ChannelFormatNiceName{ "RH", {
				{"AP_00010009", "TpFR"},
			}}},
			{"AC_00010024", ChannelFormatNiceName{ "Lsc" }},
			{"AC_00010025", ChannelFormatNiceName{ "Rsc" }},
			{"AC_00010026", ChannelFormatNiceName{ "FLM" }},
			{"AC_00010027", ChannelFormatNiceName{ "FRM" }},
			{"AC_00010028", ChannelFormatNiceName{ "CH" }},
		};
		/* clang-format on */

	};

	class PackFormatData {
	public:
		PackFormatData(std::shared_ptr<adm::AudioPackFormat> apf);
		~PackFormatData();

		int idValue{ 0 };
		std::string fullId;
		std::string name;
		std::string niceName;
		std::shared_ptr<adm::AudioPackFormat> packFormat;
		std::vector<std::shared_ptr<ChannelFormatData>> relatedChannelFormats;

	private:
		std::string makeNicePackFormatName(const std::string& originalName);
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