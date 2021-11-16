#include "admmetadata.h"
#include <bw64/bw64.hpp>
#include <adm/document.hpp>
#include <adm/parse.hpp>
#include <sstream>

using namespace admplug;

namespace {

void setUidReference(adm::Document& doc,
                     adm::AudioTrackUid& uid,
                     adm::AudioChannelFormatId elementId) {
    // Specific function to provide 2076-2 structure support even though we're only supporting -1 atm
    /*
    When a CHNA chunk of a 2076-2 file is processed, we may find references to ChannelFormat elements
     where we would normally expect to see a TrackFormat reference.
    This is because, for PCM audio, the TrackFormat and StreamFormat elements are largely redundant,
     so 2076-2 allows us to omit those elements and reference the ChannelFormat directly.
    Because the rest of the EPS has been built with 2076-1 references and expected structures in mind,
     it is easiest to workaround this issue by filling in these missing elements,
     thus converting to a 2076-1-like structure.
    */
    auto cf = doc.lookup(elementId);
    if(cf) {
        auto tf = adm::AudioTrackFormat::create(adm::AudioTrackFormatName("AudioTrackFormat"), adm::FormatDefinition::PCM);
        auto sf = adm::AudioStreamFormat::create(adm::AudioStreamFormatName("AudioStreamFormat"), adm::FormatDefinition::PCM);
        uid.setReference(tf);
        tf->setReference(sf);
        sf->setReference(cf);
    }
}

template <typename T>
void setUidReference(adm::Document& doc,
                     adm::AudioTrackUid& uid,
                     T elementId) {
    auto element = doc.lookup(elementId);
    if(element) {
        uid.setReference(element);
    }
}

void setUidReferenceUsingIdStr(adm::Document& doc,
                            adm::AudioTrackUid& uid,
                            std::string& elementIdStr) {
    if(elementIdStr.rfind("AT_", 0) == 0) {
        auto trackFormatId = adm::parseAudioTrackFormatId(elementIdStr);
        setUidReference(doc, uid, trackFormatId);
    } else if(elementIdStr.rfind("AC_", 0) == 0) {
        std::string cfIdStr = elementIdStr.substr(0, 11); // counter portion does not apply to channelformat ID's
        auto channelFormatId = adm::parseAudioChannelFormatId(cfIdStr);
        setUidReference(doc, uid, channelFormatId);
    } else if(elementIdStr.rfind("AP_", 0) == 0) {
        auto packFormatId = adm::parseAudioPackFormatId(elementIdStr);
        setUidReference(doc, uid, packFormatId);
    } else {
        auto msg = std::string("Unexpected ID: ");
        msg += elementIdStr;
        throw std::runtime_error(msg);
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
                setUidReferenceUsingIdStr(*document, *admUid, id.trackRef()); // note could be channelformat
                setUidReferenceUsingIdStr(*document, *admUid, id.packRef());
            } catch(std::runtime_error const& e) {
                // Prepend CHNA to error messages so we know the issue is related to CHNA, not AXML
                std::string msg("CHNA: ");
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
