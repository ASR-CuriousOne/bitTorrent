#include <client/torrentSession.hpp>
#include <client/networkEngine.hpp>
#include <iostream>
#include <logger/logger.hpp>
#include <string>
#include <memory>

int main(int argc, char *argv[]) {

  std::span<char *> args(argv, argc);
	
	std::string hostname, port;


  if (args.size() < 2) {
    std::cerr << "Tracker hostname or port not provided \n Usage ./client.out "
                 "<hostname> <port>"
              << std::endl;
    return -1;
  }
  hostname = std::string(args[1]);
  port = std::string(args[2]);
	
	BTClient::NetworkEngine networkEngine;

	std::shared_ptr<BTClient::TorrentSession> currentSession = std::make_shared<BTClient::TorrentSession>(networkEngine);
	currentSession->connectToTracker(hostname, port);
}
