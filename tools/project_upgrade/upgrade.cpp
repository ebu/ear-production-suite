#include <iostream>
#include <fstream>
#include "upgrade.h"
using namespace upgrade;
namespace {
const std::string currentCode = R"(\{ABCDEF019182FAEB45425520455053(20)\})";
static const std::vector<PluginCode> remap {
    {"EAR DirectSpeakers", "10"},
    {"EAR Object", "11"},
    {"EAR Monitoring 0\\+2\\+0", "A0"},
    {"EAR Monitoring 0\\+5\\+0", "A1"},
    {"EAR Monitoring 2\\+5\\+0", "A2"},
    {"EAR Monitoring 4\\+5\\+0", "A3"},
    {"EAR Monitoring 4\\+5\\+1", "A4"},
    {"EAR Monitoring 3\\+7\\+0", "A5"},
    {"EAR Monitoring 4\\+9\\+0", "A6"},
    {"EAR Monitoring 9\\+10\\+3", "A7"},
    {"EAR Monitoring 0\\+7\\+0", "A8"},
    {"EAR Monitoring 4\\+7\\+0", "A9"},
    {"EAR Scene", "FF"},
};
}

upgrade::PluginCode::PluginCode(std::string const& name, std::string new_code)
    : regex{name + ".*" + currentCode }, new_code{std::move(new_code)} {}

std::optional<std::string> upgrade::PluginCode::replace(const std::string &input) const {
  std::smatch matches;
  std::regex_search(input, matches, regex);
  if(matches.size() != 2) {
    return {};
  }
  return std::string(input.begin(), matches[1].first) + new_code + std::string(matches[1].second, input.end());
}

int upgrade::upgrade_project(int argc, char **argv) {
  if(argc != 3) {
    std::cout << "project_upgrade: upgrade reaper projects from 0.6.0 EPS format to 0.7.0 format\n"
                 "Usage: project_upgrade input_project.RPP output_project.RPP\n";
    return 1;
  }

  {
    std::ifstream outputExists{argv[2]};
    if(outputExists.good()) {
      std::cout << "Error, Output project file " << argv[2] << " already exists\n.";
      return 1;
    }
  }

  auto input = std::ifstream{argv[1]};
  if(!input.good()) {
    std::cout << "Error opening input project file " << argv[1] << "\n";
    return 1;
  }

  auto output = std::ofstream{argv[2]};
  if(!output.good()) {
    std::cout << "Error opening " << argv[2] << " for output, check you have permission\n";
    return 1;
  }
  auto count = upgrade(input, output);
  std::cout << count << " plugin instances replaced\n";
  return 0;
}

int upgrade::upgrade(std::istream &input, std::ostream &output) {
  output << std::noskipws;
  std::string line;
  int replacementCount{0};
  while(std::getline(input, line)) {
    std::optional<std::string> replaced;
    for(auto const& code_map : remap) {
      replaced = code_map.replace(line);
      if(replaced) {
        output << *replaced << '\n';
        ++replacementCount;
        break;
      }
    }
    if(!replaced) {
      output << line << '\n';
    }
  }
  return replacementCount;
}

