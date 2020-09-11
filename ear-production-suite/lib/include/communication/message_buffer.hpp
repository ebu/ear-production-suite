#pragma once
#include <vector>
#include "nng-cpp/buffer.hpp"

namespace ear {
namespace plugin {
namespace communication {
// this is essentially a stub to define the interface how we can get buffers for
// message encoding. in the long run, we don't want to use std::vector, but
// instead to something our transport layer provides/can use easily
using MessageBuffer = nng::Buffer;
inline MessageBuffer allocBuffer(std::size_t size) {
  return nng::Buffer::alloc(size);
};

}  // namespace communication
}  // namespace plugin
}  // namespace ear
