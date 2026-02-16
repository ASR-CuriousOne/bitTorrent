#include <arpa/inet.h>
#include <array>
#include <cassert>
#include <common/network.hpp>
#include <format>
#include <logger/logger.hpp>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <vector>


[[noreturn]] void inline logAndThrowFatal(const std::string &origin,
                                          const std::string &message) {
  Logger::fatal(origin, message);
  throw std::runtime_error(message);
}

namespace BTCore {

UDPConnector::UDPConnector(int port) :  m_sockv4(AF_INET,port), m_sockv6(AF_INET6, port){
  m_isRunning = true;
  m_recieverThread = std::jthread([this] { this->recieve(); });

	Logger::info(
      "UDPConnector",
      std::format("Listening on both ipv4 and ipv6 on port {}", port));
}


UDPConnector::~UDPConnector() {
  m_isRunning = false;
  assert(m_recieverThread.joinable());
}

int UDPConnector::sendTo(const std::string &host,
                                const std::string &port,
                                const std::span<const std::byte> &bytes) {
  struct addrinfo hints{.ai_family = AF_UNSPEC,
                        .ai_socktype = SOCK_DGRAM,
                        .ai_protocol = IPPROTO_UDP};
  struct addrinfo *resultList, *it;

  if (getaddrinfo(host.c_str(), port.c_str(), &hints, &resultList) != 0) {
    logAndThrowFatal("Send To Tracker", std::format("Unable to get addresultLists info for {} on port {}", host, port));
  }

  for (it = resultList; it != NULL; it = it->ai_next) {
    if (it->ai_family == AF_INET6 && m_sockv6.get() != -1) {
      Logger::debug("sendTo","Found IPv6 address sending message over IPv6.");
      if (sendto(m_sockv6.get(), bytes.data(), bytes.size(), 0, it->ai_addr,
                 it->ai_addrlen) < 0) {
        logAndThrowFatal("Send To Tracker", "Sending over IPv6 failed");
      }
      break;
    } else if (it->ai_family == AF_INET && m_sockv4.get() != -1) {
      Logger::debug("sendTo","Found IPv4 address sending message over IPv4.");
      if (sendto(m_sockv4.get(), bytes.data(), bytes.size(), 0, it->ai_addr,
                 it->ai_addrlen) < 0) {
        logAndThrowFatal("Send To Tracker", "Sending over IPv4 failed");
      }
      break;
    }
  }

  freeaddrinfo(resultList);
  return 0;
}

void UDPConnector::setOnReceive(ReceiveCallback cb){
	m_onReceive = std::move(cb);
}

void UDPConnector::recieve() {
  std::array<struct pollfd, 2> fds;

  fds[0] = {m_sockv4.get(), POLLIN, 0};
  fds[1] = {m_sockv6.get(), POLLIN, 0};

  std::vector<char> buffer(4096);

  while (m_isRunning) {
    int ret = ::poll(fds.data(), fds.size(), 1000);

    if (ret > 0) {
      for (const auto &item : fds) {
        if (item.revents & POLLIN) {
          handleRead(item.fd, buffer);
        }
      }
    }
  }
}

void UDPConnector::handleRead(int fd, std::vector<char> &buffer) {
  struct sockaddr_storage sender;
  socklen_t len = sizeof(sender);

  ssize_t n = ::recvfrom(fd, buffer.data(), buffer.size(), 0,
                         (struct sockaddr *)&sender, &len);

  if (n > 0)
    Logger::info("handle Read", std::format("Received {} bytes from sender: {}",
                                            n, sender.ss_family));
	if(m_onReceive){
		Packet p;
		p.payload.assign(buffer.begin(),buffer.end());
		p.sender = sender;

		m_onReceive(std::move(p));
	}
}
} // namespace BTCore
