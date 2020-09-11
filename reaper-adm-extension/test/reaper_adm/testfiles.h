#ifndef TESTFILES_H
#define TESTFILES_H

#include <bw64/bw64.hpp>
#include <adm/parse.hpp>
#include <adm/write.hpp>
#include <adm/common_definitions.hpp>
#include <sstream>
#include <tuple>

inline std::pair<std::shared_ptr<adm::Document>, std::shared_ptr<bw64::ChnaChunk>> admDataFrom(std::string fileName) {
    bw64::Bw64Reader reader{fileName.c_str()};
    std::stringstream ss;
    reader.axmlChunk()->write(ss);
    auto doc = adm::parseXml(ss);
    return std::make_pair(doc, reader.chnaChunk());
}


#endif // TESTFILES_H
