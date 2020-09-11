#include "nng-cpp/error_handling.hpp"
#include <nng/nng.h>
#include <system_error>

namespace std {
template <>
struct is_error_code_enum<nng_errno_enum> : public std::true_type {};
}  // namespace std

namespace {
class ErrorCategory : public std::error_category {
  const char* name() const noexcept override { return "nng"; }
  std::string message(int condition) const override {
    return nng_strerror(condition);
  }
};

const ErrorCategory errorCategoryInstance{};
}  // anonymous namespace

namespace nng {
std::error_code makeErrorCode(int value) {
  return std::error_code(value, errorCategoryInstance);
}
}  // namespace nng
