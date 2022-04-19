#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace ear {
namespace plugin {
namespace ui {

struct IsoLanguageTriplet {
  std::string alpha3;
  std::string alpha2;
  std::string english;
};

static const std::vector<IsoLanguageTriplet> LANGUAGES{
    IsoLanguageTriplet{"aar", "aa", "Afar"},
    IsoLanguageTriplet{"abk", "ab", "Abkhazian"},
    IsoLanguageTriplet{"afr", "af", "Afrikaans"},
    IsoLanguageTriplet{"aka", "ak", "Akan"},
    IsoLanguageTriplet{"alb", "sq", "Albanian"},
    IsoLanguageTriplet{"amh", "am", "Amharic"},
    IsoLanguageTriplet{"ara", "ar", "Arabic"},
    IsoLanguageTriplet{"arg", "an", "Aragonese"},
    IsoLanguageTriplet{"arm", "hy", "Armenian"},
    IsoLanguageTriplet{"asm", "as", "Assamese"},
    IsoLanguageTriplet{"ava", "av", "Avaric"},
    IsoLanguageTriplet{"ave", "ae", "Avestan"},
    IsoLanguageTriplet{"aym", "ay", "Aymara"},
    IsoLanguageTriplet{"aze", "az", "Azerbaijani"},
    IsoLanguageTriplet{"bak", "ba", "Bashkir"},
    IsoLanguageTriplet{"bam", "bm", "Bambara"},
    IsoLanguageTriplet{"baq", "eu", "Basque"},
    IsoLanguageTriplet{"bel", "be", "Belarusian"},
    IsoLanguageTriplet{"ben", "bn", "Bengali"},
    IsoLanguageTriplet{"bih", "bh", "Bihari languages"},
    IsoLanguageTriplet{"bis", "bi", "Bislama"},
    IsoLanguageTriplet{"bos", "bs", "Bosnian"},
    IsoLanguageTriplet{"bre", "br", "Breton"},
    IsoLanguageTriplet{"bul", "bg", "Bulgarian"},
    IsoLanguageTriplet{"bur", "my", "Burmese"},
    IsoLanguageTriplet{"cat", "ca", "Catalan; Valencian"},
    IsoLanguageTriplet{"cha", "ch", "Chamorro"},
    IsoLanguageTriplet{"che", "ce", "Chechen"},
    IsoLanguageTriplet{"chi", "zh", "Chinese"},
    IsoLanguageTriplet{
        "chu", "cu",
        "Church Slavic; Old Slavonic; Church Slavonic; Old Bulgarian; Old "
        "Church Slavonic"},
    IsoLanguageTriplet{"chv", "cv", "Chuvash"},
    IsoLanguageTriplet{"cor", "kw", "Cornish"},
    IsoLanguageTriplet{"cos", "co", "Corsican"},
    IsoLanguageTriplet{"cre", "cr", "Cree"},
    IsoLanguageTriplet{"cze", "cs", "Czech"},
    IsoLanguageTriplet{"dan", "da", "Danish"},
    IsoLanguageTriplet{"div", "dv", "Divehi; Dhivehi; Maldivian"},
    IsoLanguageTriplet{"dut", "nl", "Dutch; Flemish"},
    IsoLanguageTriplet{"dzo", "dz", "Dzongkha"},
    IsoLanguageTriplet{"eng", "en", "English"},
    IsoLanguageTriplet{"epo", "eo", "Esperanto"},
    IsoLanguageTriplet{"est", "et", "Estonian"},
    IsoLanguageTriplet{"ewe", "ee", "Ewe"},
    IsoLanguageTriplet{"fao", "fo", "Faroese"},
    IsoLanguageTriplet{"fij", "fj", "Fijian"},
    IsoLanguageTriplet{"fin", "fi", "Finnish"},
    IsoLanguageTriplet{"fre", "fr", "French"},
    IsoLanguageTriplet{"fry", "fy", "Western Frisian"},
    IsoLanguageTriplet{"ful", "ff", "Fulah"},
    IsoLanguageTriplet{"geo", "ka", "Georgian"},
    IsoLanguageTriplet{"ger", "de", "German"},
    IsoLanguageTriplet{"gla", "gd", "Gaelic; Scottish Gaelic"},
    IsoLanguageTriplet{"gle", "ga", "Irish"},
    IsoLanguageTriplet{"glg", "gl", "Galician"},
    IsoLanguageTriplet{"glv", "gv", "Manx"},
    IsoLanguageTriplet{"gre", "el", "Greek, Modern (1453-)"},
    IsoLanguageTriplet{"grn", "gn", "Guarani"},
    IsoLanguageTriplet{"guj", "gu", "Gujarati"},
    IsoLanguageTriplet{"hat", "ht", "Haitian; Haitian Creole"},
    IsoLanguageTriplet{"hau", "ha", "Hausa"},
    IsoLanguageTriplet{"heb", "he", "Hebrew"},
    IsoLanguageTriplet{"her", "hz", "Herero"},
    IsoLanguageTriplet{"hin", "hi", "Hindi"},
    IsoLanguageTriplet{"hmo", "ho", "Hiri Motu"},
    IsoLanguageTriplet{"hrv", "hr", "Croatian"},
    IsoLanguageTriplet{"hun", "hu", "Hungarian"},
    IsoLanguageTriplet{"ibo", "ig", "Igbo"},
    IsoLanguageTriplet{"ice", "is", "Icelandic"},
    IsoLanguageTriplet{"ido", "io", "Ido"},
    IsoLanguageTriplet{"iii", "ii", "Sichuan Yi; Nuosu"},
    IsoLanguageTriplet{"iku", "iu", "Inuktitut"},
    IsoLanguageTriplet{"ile", "ie", "Interlingue; Occidental"},
    IsoLanguageTriplet{
        "ina", "ia",
        "Interlingua (International Auxiliary Language Association)"},
    IsoLanguageTriplet{"ind", "id", "Indonesian"},
    IsoLanguageTriplet{"ipk", "ik", "Inupiaq"},
    IsoLanguageTriplet{"ita", "it", "Italian"},
    IsoLanguageTriplet{"jav", "jv", "Javanese"},
    IsoLanguageTriplet{"jpn", "ja", "Japanese"},
    IsoLanguageTriplet{"kal", "kl", "Kalaallisut; Greenlandic"},
    IsoLanguageTriplet{"kan", "kn", "Kannada"},
    IsoLanguageTriplet{"kas", "ks", "Kashmiri"},
    IsoLanguageTriplet{"kau", "kr", "Kanuri"},
    IsoLanguageTriplet{"kaz", "kk", "Kazakh"},
    IsoLanguageTriplet{"khm", "km", "Central Khmer"},
    IsoLanguageTriplet{"kik", "ki", "Kikuyu; Gikuyu"},
    IsoLanguageTriplet{"kin", "rw", "Kinyarwanda"},
    IsoLanguageTriplet{"kir", "ky", "Kirghiz; Kyrgyz"},
    IsoLanguageTriplet{"kom", "kv", "Komi"},
    IsoLanguageTriplet{"kon", "kg", "Kongo"},
    IsoLanguageTriplet{"kor", "ko", "Korean"},
    IsoLanguageTriplet{"kua", "kj", "Kuanyama; Kwanyama"},
    IsoLanguageTriplet{"kur", "ku", "Kurdish"},
    IsoLanguageTriplet{"lao", "lo", "Lao"},
    IsoLanguageTriplet{"lat", "la", "Latin"},
    IsoLanguageTriplet{"lav", "lv", "Latvian"},
    IsoLanguageTriplet{"lim", "li", "Limburgan; Limburger; Limburgish"},
    IsoLanguageTriplet{"lin", "ln", "Lingala"},
    IsoLanguageTriplet{"lit", "lt", "Lithuanian"},
    IsoLanguageTriplet{"ltz", "lb", "Luxembourgish; Letzeburgesch"},
    IsoLanguageTriplet{"lub", "lu", "Luba-Katanga"},
    IsoLanguageTriplet{"lug", "lg", "Ganda"},
    IsoLanguageTriplet{"mac", "mk", "Macedonian"},
    IsoLanguageTriplet{"mah", "mh", "Marshallese"},
    IsoLanguageTriplet{"mal", "ml", "Malayalam"},
    IsoLanguageTriplet{"mao", "mi", "Maori"},
    IsoLanguageTriplet{"mar", "mr", "Marathi"},
    IsoLanguageTriplet{"may", "ms", "Malay"},
    IsoLanguageTriplet{"mlg", "mg", "Malagasy"},
    IsoLanguageTriplet{"mlt", "mt", "Maltese"},
    IsoLanguageTriplet{"mon", "mn", "Mongolian"},
    IsoLanguageTriplet{"nau", "na", "Nauru"},
    IsoLanguageTriplet{"nav", "nv", "Navajo; Navaho"},
    IsoLanguageTriplet{"nbl", "nr", "Ndebele, South; South Ndebele"},
    IsoLanguageTriplet{"nde", "nd", "Ndebele, North; North Ndebele"},
    IsoLanguageTriplet{"ndo", "ng", "Ndonga"},
    IsoLanguageTriplet{"nep", "ne", "Nepali"},
    IsoLanguageTriplet{"nno", "nn", "Norwegian Nynorsk; Nynorsk, Norwegian"},
    IsoLanguageTriplet{"nob", "nb", "Bokmål, Norwegian; Norwegian Bokmål"},
    IsoLanguageTriplet{"nor", "no", "Norwegian"},
    IsoLanguageTriplet{"nya", "ny", "Chichewa; Chewa; Nyanja"},
    IsoLanguageTriplet{"oci", "oc", "Occitan (post 1500); Provençal"},
    IsoLanguageTriplet{"oji", "oj", "Ojibwa"},
    IsoLanguageTriplet{"ori", "or", "Oriya"},
    IsoLanguageTriplet{"orm", "om", "Oromo"},
    IsoLanguageTriplet{"oss", "os", "Ossetian; Ossetic"},
    IsoLanguageTriplet{"pan", "pa", "Panjabi; Punjabi"},
    IsoLanguageTriplet{"per", "fa", "Persian"},
    IsoLanguageTriplet{"pli", "pi", "Pali"},
    IsoLanguageTriplet{"pol", "pl", "Polish"},
    IsoLanguageTriplet{"por", "pt", "Portuguese"},
    IsoLanguageTriplet{"pus", "ps", "Pushto; Pashto"},
    IsoLanguageTriplet{"que", "qu", "Quechua"},
    IsoLanguageTriplet{"roh", "rm", "Romansh"},
    IsoLanguageTriplet{"rum", "ro", "Romanian; Moldavian; Moldovan"},
    IsoLanguageTriplet{"run", "rn", "Rundi"},
    IsoLanguageTriplet{"rus", "ru", "Russian"},
    IsoLanguageTriplet{"sag", "sg", "Sango"},
    IsoLanguageTriplet{"san", "sa", "Sanskrit"},
    IsoLanguageTriplet{"sin", "si", "Sinhala; Sinhalese"},
    IsoLanguageTriplet{"slo", "sk", "Slovak"},
    IsoLanguageTriplet{"slv", "sl", "Slovenian"},
    IsoLanguageTriplet{"sme", "se", "Northern Sami"},
    IsoLanguageTriplet{"smo", "sm", "Samoan"},
    IsoLanguageTriplet{"sna", "sn", "Shona"},
    IsoLanguageTriplet{"snd", "sd", "Sindhi"},
    IsoLanguageTriplet{"som", "so", "Somali"},
    IsoLanguageTriplet{"sot", "st", "Sotho, Southern"},
    IsoLanguageTriplet{"spa", "es", "Spanish; Castilian"},
    IsoLanguageTriplet{"srd", "sc", "Sardinian"},
    IsoLanguageTriplet{"srp", "sr", "Serbian"},
    IsoLanguageTriplet{"ssw", "ss", "Swati"},
    IsoLanguageTriplet{"sun", "su", "Sundanese"},
    IsoLanguageTriplet{"swa", "sw", "Swahili"},
    IsoLanguageTriplet{"swe", "sv", "Swedish"},
    IsoLanguageTriplet{"tah", "ty", "Tahitian"},
    IsoLanguageTriplet{"tam", "ta", "Tamil"},
    IsoLanguageTriplet{"tat", "tt", "Tatar"},
    IsoLanguageTriplet{"tel", "te", "Telugu"},
    IsoLanguageTriplet{"tgk", "tg", "Tajik"},
    IsoLanguageTriplet{"tgl", "tl", "Tagalog"},
    IsoLanguageTriplet{"tha", "th", "Thai"},
    IsoLanguageTriplet{"tib", "bo", "Tibetan"},
    IsoLanguageTriplet{"tir", "ti", "Tigrinya"},
    IsoLanguageTriplet{"ton", "to", "Tonga (Tonga Islands)"},
    IsoLanguageTriplet{"tsn", "tn", "Tswana"},
    IsoLanguageTriplet{"tso", "ts", "Tsonga"},
    IsoLanguageTriplet{"tuk", "tk", "Turkmen"},
    IsoLanguageTriplet{"tur", "tr", "Turkish"},
    IsoLanguageTriplet{"twi", "tw", "Twi"},
    IsoLanguageTriplet{"uig", "ug", "Uighur; Uyghur"},
    IsoLanguageTriplet{"ukr", "uk", "Ukrainian"},
    IsoLanguageTriplet{"urd", "ur", "Urdu"},
    IsoLanguageTriplet{"uzb", "uz", "Uzbek"},
    IsoLanguageTriplet{"ven", "ve", "Venda"},
    IsoLanguageTriplet{"vie", "vi", "Vietnamese"},
    IsoLanguageTriplet{"vol", "vo", "Volapük"},
    IsoLanguageTriplet{"wel", "cy", "Welsh"},
    IsoLanguageTriplet{"wln", "wa", "Walloon"},
    IsoLanguageTriplet{"wol", "wo", "Wolof"},
    IsoLanguageTriplet{"xho", "xh", "Xhosa"},
    IsoLanguageTriplet{"yid", "yi", "Yiddish"},
    IsoLanguageTriplet{"yor", "yo", "Yoruba"},
    IsoLanguageTriplet{"zha", "za", "Zhuang; Chuang"},
    IsoLanguageTriplet{"zul", "zu", "Zulu"}};

inline int getIndexForAlpha3(const std::string& alpha3) {
  auto it = std::find_if(
      LANGUAGES.begin(), LANGUAGES.end(),
      [&alpha3](auto const& language) { return language.alpha3 == alpha3; });
  if (it != LANGUAGES.end()) {
    return std::distance(LANGUAGES.begin(), it);
  }
  return -1;
}

inline int getIndexForAlpha2(const std::string& alpha2) {
  auto it = std::find_if(
      LANGUAGES.begin(), LANGUAGES.end(),
      [&alpha2](auto const& language) { return language.alpha2 == alpha2; });
  if (it != LANGUAGES.end()) {
    return std::distance(LANGUAGES.begin(), it);
  }
  return -1;
}

inline int getIndexForAlphaN(const std::string& alpha) {
    if(alpha.length() == 2) return getIndexForAlpha2(alpha);
    if(alpha.length() == 3) return getIndexForAlpha3(alpha);
    return -1;
}

inline int getLanguageIndex(const std::string& language) {
    auto index = getIndexForAlphaN(language);
    if(index >= 0) return index;
    if(auto it = std::find_if(LANGUAGES.begin(), LANGUAGES.end(), [&language](auto const& triplet) {
        return triplet.english == language;
    }); it != LANGUAGES.end()) {
        return static_cast<int>(std::distance(LANGUAGES.begin(), it));
    }
    return -1;
}

}  // namespace ui
}  // namespace plugin
}  // namespace ear
