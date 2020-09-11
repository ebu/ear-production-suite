#pragma once
#include <string>
#include <memory>

namespace adm {
  class Document;
}

namespace bw64 {
  class ChnaChunk;
  class AxmlChunk;
}

namespace admplug {

class IADMMetaData {
public:
    virtual ~IADMMetaData() = default;
    virtual std::shared_ptr<const bw64::ChnaChunk> chna() const = 0;
    virtual std::shared_ptr<const bw64::AxmlChunk> axml() const = 0;
    virtual std::shared_ptr<const adm::Document> adm() const = 0;
    virtual std::string fileName() const = 0;
};

class ADMMetaData : public IADMMetaData
{
public:
    ADMMetaData(std::string fileName);
    std::shared_ptr<const bw64::ChnaChunk> chna() const override;
    std::shared_ptr<const bw64::AxmlChunk> axml() const override;
    std::shared_ptr<const adm::Document> adm() const override;
    std::string fileName() const override;

private:
    std::shared_ptr<bw64::ChnaChunk> chnaChunk;
    std::shared_ptr<bw64::AxmlChunk> axmlChunk;
    std::shared_ptr<adm::Document> document;
    std::string name;
    void parseMetadata();
    void completeUidReferences();
};

}
