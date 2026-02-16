#pragma once
#include <atomic>
#include <cstddef>
#include <functional>
#include <span>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <vector>

namespace BTCore {

class UDPSocket {
  int m_fd = -1;
	int m_family = AF_INET;
	int m_port;

public:
  UDPSocket(int af);
	UDPSocket(int af, int port);

  UDPSocket(const UDPSocket &) = delete;
  UDPSocket &operator=(const UDPSocket &) = delete;

  UDPSocket(UDPSocket &&other) noexcept : m_fd(other.m_fd) { other.m_fd = -1; }

  ~UDPSocket();

	int get(){return m_fd;}
	int getPort(){return m_port;}

	void bindToPort(int port);

};

struct Packet{
	std::vector<char> payload;
	struct sockaddr_storage sender;
};

using ReceiveCallback = std::function<void(Packet)>;

class UDPConnector {
private:
  std::atomic<bool> m_isRunning = false;
  std::jthread m_recieverThread;

	ReceiveCallback m_onReceive;

 	UDPSocket m_sockv4, m_sockv6;

  void recieve();
  void handleRead(int fd, std::vector<char> &buffer);

public:
  UDPConnector(int port = 6881);

  ~UDPConnector();

  int sendTo(const std::string &host, const std::string &port,
             const std::span<const std::byte> &bytes);

	void setOnReceive(ReceiveCallback cb);

	int getPort(){return m_sockv4.getPort();}
};
} // namespace BTCore
