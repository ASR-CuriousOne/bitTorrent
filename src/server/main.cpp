#include <common/bencoding.hpp>
#include <common/sha1.hpp>
#include <common/utils.hpp>
#include <iostream>
#include <logger/logger.hpp>
#include <common/bencoding.hpp>

int main(int argc, char *argv[]) {
  std::span<char *> args(argv, argc);

  if (args.size() < 2) {
    std::cerr << "Provide atleast one argument: ./server.out <filepath>"
              << std::endl;
    return -1;
  }

  std::string filepath = std::string(args[1]);


}
