#include <common/bencoding.hpp>
#include <common/sha1.hpp>
#include <common/utils.hpp>
#include <iostream>
#include <logger/logger.hpp>

int main(int argc, char *argv[]) {
  std::span<char *> args(argv, argc);

  if (args.size() < 2) {
    std::cerr << "Provide atleast one argument: ./server.out <input>"
              << std::endl;
    return -1;
  }

  std::string input = std::string(args[1]);

  BTCore::SHA1 sha1;
  sha1.update(input);
  sha1.finalize();

  auto digest = sha1.getDigest();
  Logger::info("SHA1", std::format("Calculated InfoHash: {}",
                                   BTCore::Utils::toHex(digest)));
}
