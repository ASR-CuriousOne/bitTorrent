#pragma once
#include <common/network.hpp>
#include <common/queue.hpp>
#include <common/transIdGen.hpp>
#include <string>

namespace BTClient {
class Client {
private:
  const int m_port = 6881;
  BTCore::UDPConnector m_udpCon;
  BTCore::mutexQueue<BTCore::Packet> m_responseQueue;

	BTCore::TransIdGen m_transIdGen;

public:
  Client(int port = 6881);

  void run();

  void connectToTracker(const std::string &hostname, const std::string &port);
};
} // namespace BTClient
