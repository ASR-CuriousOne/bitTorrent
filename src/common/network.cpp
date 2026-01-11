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
UDPConnector::UDPConnector(int port) : m_port(port) {
  initSockets();

  m_isRunning = true;
  m_recieverThread = std::jthread([this] { this->recieve(); });
}

void UDPConnector::initSockets() {
  m_sockv4 = socket(AF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in addr4 = {};
  addr4.sin_family = AF_INET;
  addr4.sin_port = htons(m_port);
  addr4.sin_addr.s_addr = INADDR_ANY;

  if (bind(m_sockv4, reinterpret_cast<struct sockaddr *>(&addr4),
           sizeof(addr4)) != 0)
    logAndThrowFatal("UDPConnector", "IPv4 Port binding failed");

  m_sockv6 = socket(AF_INET6, SOCK_DGRAM, 0);
  int on = 1;
  setsockopt(m_sockv6, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));

  struct sockaddr_in6 addr6 = {};
  addr6.sin6_family = AF_INET6;
  addr6.sin6_port = htons(m_port);
  addr6.sin6_addr = in6addr_any;

  if (bind(m_sockv6, reinterpret_cast<struct sockaddr *>(&addr6),
           sizeof(addr6)) != 0)
    logAndThrowFatal("UDPConnector", "IPv6 Port binding failed");

  Logger::info(
      "UDPConnector",
      std::format("Listening on both ipv4 and ipv6 on port {}", m_port));
}

UDPConnector::~UDPConnector() {
  m_isRunning = false;
  assert(m_recieverThread.joinable());

  close(m_sockv4);
  close(m_sockv6);
}

int UDPConnector::sendToTracker(const std::string &host,
                                const std::string &port,
                                std::span<std::byte> bytes) {
  struct addrinfo hints{.ai_family = AF_UNSPEC,
                        .ai_socktype = SOCK_DGRAM,
                        .ai_protocol = IPPROTO_UDP};
  struct addrinfo *resultList, *it;

  if (getaddrinfo(host.c_str(), port.c_str(), &hints, &resultList) != 0) {
    logAndThrowFatal("Send To Tracker", "Unable to get addresultLists info");
  }

  for (it = resultList; it != NULL; it = it->ai_next) {
    if (it->ai_family == AF_INET6 && m_sockv6 != -1) {
      Logger::debug("Found IPv6 address sending message over IPv6.");
      if (sendto(m_sockv6, bytes.data(), bytes.size(), 0, it->ai_addr,
                 it->ai_addrlen) != 0) {
        logAndThrowFatal("Send To Tracker", "Sending over IPv6 failed");
      }
      break;
    } else if (it->ai_family == AF_INET && m_sockv4 != -1) {
      Logger::debug("Found IPv4 address sending message over IPv4.");
      if (sendto(m_sockv4, bytes.data(), bytes.size(), 0, it->ai_addr,
                 it->ai_addrlen) != 0) {
        logAndThrowFatal("Send To Tracker", "Sending over IPv4 failed");
      }
      break;
    }
  }

  freeaddrinfo(resultList);
  return 0;
}

void UDPConnector::recieve() {
  std::array<struct pollfd, 2> fds;

  fds[0] = {m_sockv4, POLLIN, 0};
  fds[1] = {m_sockv6, POLLIN, 0};

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
}
} // namespace BTCore
