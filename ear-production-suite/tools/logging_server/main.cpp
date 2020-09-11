#include "detail/log_config.hpp"
#include "nng-cpp/nng.hpp"
#include <array>
#include <iostream>
#include <string>

const std::size_t BUFFER_SIZE = 4096;

int main(int argc, char** argv) {
  try {
    nng::PullSocket socket;
    socket.listen(ear::plugin::detail::DEFAULT_LOG_ENDPOINT);

    std::array<char, BUFFER_SIZE> buf;
    for (;;) {
      auto buffer = socket.read();
      std::string msg((char*)buffer.data(), buffer.size());
      std::cerr << msg << std::flush;
    }
  } catch (const std::system_error& ec) {
    std::cout << ec.code() << " " << ec.what() << std::endl;
  }
  return 0;
}
