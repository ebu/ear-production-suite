#include "log.hpp"
#include "nng-cpp/protocols/push.hpp"
#include <array>
#include <iostream>

int main(int argc, char** argv) {
  std::cout << "please enter text, it will be send to the logger "
               "..."
            << std::endl;
  auto logger = ear::plugin::createLogger("console test logger");
  std::string message;
  while (true) {
    std::getline(std::cin, message);
    if (!std::cin.good()) {
      return 0;
    }
    logger->info(message);
  }
}
