#include <client/networkEngine.hpp>
#include <client/torrentSession.hpp>
#include <iostream>
#include <logger/logger.hpp>
#include <memory>
#include <string>

Logger::Logger g_logger;

int main(int argc, char *argv[]) {

  std::span<char *> args(argv, argc);

  std::string hostname, port;

  if (args.size() < 1) {
    std::cerr << "Torrent File not provided \n Usage ./client.out "
                 "<filename>"
              << std::endl;
    return -1;
  }

  std::filesystem::path torrentFilePath(args[1]);

	Logger::Logger generalLogger;

  BTClient::NetworkEngine networkEngine(generalLogger,69000);

  std::shared_ptr<BTClient::TorrentSession> currentSession =
      std::make_shared<BTClient::TorrentSession>(generalLogger,networkEngine);
  currentSession->startTorrent(torrentFilePath);
}
