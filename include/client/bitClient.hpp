#pragma once
#include <string>

namespace BTClient {
class Client {
private:
  const int m_port = 6881;

public:
  Client(int port = 6881);

  void run();

  void connectToTracker(const std::string &hostname, const std::string &port);
};
} // namespace BTClient
