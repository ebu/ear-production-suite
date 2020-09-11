#include "mockup_scene.hpp"
#include <spdlog/spdlog.h>
#include <memory>

namespace ear {

SceneMockup::SceneMockup() { spdlog::info("SceneMockup plugin created"); }

SceneMockup::~SceneMockup() { spdlog::info("SceneMockup plugin destroyed"); }

}  // namespace ear
