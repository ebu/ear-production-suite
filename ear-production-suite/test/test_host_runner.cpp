#include "mockup_host.hpp"
#include <spdlog/spdlog.h>

int main() {
  spdlog::set_level(spdlog::level::trace);
  ear::MockupHost host;

  host.insertScenePlugin();
  return 0;
}
