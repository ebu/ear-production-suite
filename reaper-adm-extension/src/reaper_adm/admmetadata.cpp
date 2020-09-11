#include "admmetadata.h"
#include <bw64/bw64.hpp>
#include <adm/document.hpp>
#include <adm/parse.hpp>
#include <sstream>

using namespace admplug;

namespace {
template <typename T>
void setUidReference(adm::Document& doc,
                     adm::AudioTrackUid& uid,
                     T elementId) {
    auto element = doc.lookup(elementId);
    if(element) {
        uid.setReference(element);
    }
}

}

void ADMMetaData::parseMetadata()
{
    auto reader = bw64::readFile(name.c_str());

    chnaChunk = reader->chnaChunk();
    axmlChunk = reader->axmlChunk();
    if (chnaChunk && axmlChunk) {
      std::stringstream xmlStream;
      axmlChunk->write(xmlStream);
      auto parsedDocument = adm::parseXml(xmlStream, adm::xml::ParserOptions::recursive_node_search);
      document = parsedDocument->deepCopy();
    }
}

void ADMMetaData::completeUidReferences()
{
    auto chnaIds = chnaChunk->audioIds();
    for(auto id : chnaIds) {
        auto uidId = adm::parseAudioTrackUidId(id.uid());
        auto admUid = document->lookup(uidId);
        if(admUid) {
            try {
                auto trackFormatId = adm::parseAudioTrackFormatId(id.trackRef());
                setUidReference(*document, *admUid, trackFormatId);
                auto packFormatId = adm::parseAudioPackFormatId(id.packRef());
                setUidReference(*document, *admUid, packFormatId);
            } catch (std::runtime_error const& e) {
                // Prepend CHNA to error messages so we know the issue is related to CHNA, not AXML
                auto msg = std::string("CHNA: ");
                msg += e.what();
                throw std::runtime_error(msg);
            }
        }
    }
}

admplug::ADMMetaData::ADMMetaData(std::string file) : name{file}
{
    parseMetadata();
    if(document) completeUidReferences();
}


std::shared_ptr<bw64::ChnaChunk const> admplug::ADMMetaData::chna() const
{
    return chnaChunk;
}

std::shared_ptr<bw64::AxmlChunk const> admplug::ADMMetaData::axml() const
{
  return axmlChunk;
}

std::shared_ptr<adm::Document const> admplug::ADMMetaData::adm() const
{
    return document;
}

std::string admplug::ADMMetaData::fileName() const
{
    return name;
}
