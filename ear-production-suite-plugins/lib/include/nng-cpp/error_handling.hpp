#pragma once
#include <nng/nng.h>
#include <system_error>
#include <ear-plugin-base/export.h>

namespace nng {
EAR_PLUGIN_BASE_EXPORT std::error_code makeErrorCode(int value);

class NNGError : public std::system_error {
 public:
  NNGError(const std::error_code &ec) : std::system_error(ec) {}
};

/**
 * Check error value and throw an exception if appropriate.
 *
 * If `value` is not 0, the error value will be converted to
 * an std::error_code and an exception of type std::system_error will be thrown
 */
inline void handleError(int value) {
  if (value != 0) {
    throw NNGError(makeErrorCode(value));
  }
}
}  // namespace nng
