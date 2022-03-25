#include <memory>
#include <string>

#ifdef EPS_ENABLE_LOGGING
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_OFF
#endif

#include <spdlog/spdlog.h>
#include <ear-plugin-base/export.h>

namespace spdlog {
class logger;
}

#ifdef EPS_ENABLE_LOGGING
#define EAR_LOGGER_TRACE(logger, ...)         \
  if (logger) {                               \
    SPDLOG_LOGGER_TRACE(logger, __VA_ARGS__); \
  }
#define EAR_LOGGER_DEBUG(logger, ...)         \
  if (logger) {                               \
    SPDLOG_LOGGER_DEBUG(logger, __VA_ARGS__); \
  }
#define EAR_LOGGER_INFO(logger, ...)         \
  if (logger) {                              \
    SPDLOG_LOGGER_INFO(logger, __VA_ARGS__); \
  }
#define EAR_LOGGER_WARN(logger, ...)         \
  if (logger) {                              \
    SPDLOG_LOGGER_WARN(logger, __VA_ARGS__); \
  }
#define EAR_LOGGER_ERROR(logger, ...)         \
  if (logger) {                               \
    SPDLOG_LOGGER_ERROR(logger, __VA_ARGS__); \
  }
#else
#define EAR_LOGGER_TRACE(logger, ...) (void)0
#define EAR_LOGGER_DEBUG(logger, ...) (void)0
#define EAR_LOGGER_INFO(logger, ...) (void)0
#define EAR_LOGGER_WARN(logger, ...) (void)0
#define EAR_LOGGER_ERROR(logger, ...) (void)0
#endif

namespace ear {
namespace plugin {
EAR_PLUGIN_BASE_EXPORT std::shared_ptr<spdlog::logger> createLogger(
    const std::string& name);
}
}  // namespace ear
