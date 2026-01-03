#include <common/bencoding.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <logger/logger.hpp>
#include <span>
#include <stdexcept>

int main(int argc, char *argv[]) {

  std::span<char *> args(argv, argc);

  std::filesystem::path bencodeFilePath;

  for (int i = 1; i < args.size(); ++i) {
    std::string_view arg = args[i];
    if (arg == "--version") {
      std::cout << "v1.0.0\n";
      return 0;
    } else {
      bencodeFilePath = std::filesystem::path(args[i]);
    }
  }

  if (bencodeFilePath.empty()) {
    throw std::runtime_error("No Bencode file provided");
    return -1;
  }
  if (!std::filesystem::exists(bencodeFilePath)) {
    throw std::runtime_error("File not found" + bencodeFilePath.string());
    return -1;
  }

  std::ifstream bencodeFile(bencodeFilePath, std::ios::binary);

  if (!bencodeFile.is_open()) {
    throw std::runtime_error("Failed to open file for reading: " +
                             bencodeFilePath.string());
  }

	std::stringstream buffer;
  buffer << bencodeFile.rdbuf();
	
	BTCore::BNode root = BTCore::BDecoder::decode(buffer.str());

	std::cout << "Decoded (JSON style):\n";
  root.printNode();
  std::cout << std::endl;
	
	
}
