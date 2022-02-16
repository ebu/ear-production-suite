#ifndef EAR_PRODUCTION_SUITE_WEAK_PTR_HELPERS_HPP
#define EAR_PRODUCTION_SUITE_WEAK_PTR_HELPERS_HPP
#include <memory>
#include <type_traits>
namespace ear {
namespace plugin {
namespace detail {

template <typename T, typename U>
[[ nodiscard ]]
std::shared_ptr<T> lockIfSame(std::weak_ptr<T> const& weak, U* raw) {
  if (raw) {
    if (auto shared = weak.lock(); shared && shared.get() == raw) {
      return shared;
    }
  }
  return {};
}

}
}
}
#endif  // EAR_PRODUCTION_SUITE_WEAK_PTR_HELPERS_HPP
