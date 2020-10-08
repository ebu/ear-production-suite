#pragma once
#include <optional>
#include <ostream>
#include <regex>
#include <vector>
#include <string>

namespace upgrade {


class PluginCode {
public:
  PluginCode(std::string const& name, std::string newCode);
  [[nodiscard]]
  std::optional<std::string> replace(std::string const& input) const;

private:
  std::regex regex;
  std::string new_code;
};

int upgrade_project(int argc, char* argv[]);
int upgrade(std::istream& input, std::ostream& output);
}