#include "log.hpp"
#include "detail/spdl_nng_sink.hpp"
#include <spdlog/logger.h>

namespace ear {
namespace plugin {
std::shared_ptr<spdlog::logger> createLogger(const std::string& name) {
  auto sink = std::make_shared<detail::NNGSink_mt>();
  return std::make_shared<spdlog::logger>(name, sink);
}
}  // namespace plugin
}  // namespace ear
