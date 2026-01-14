#include <client/bitClient.hpp>
#include <iostream>
#include <logger/logger.hpp>
#include <string>

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

	BTClient::Client client;
	client.connectToTracker(hostname, port);
}
