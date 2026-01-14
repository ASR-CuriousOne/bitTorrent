#include <common/network.hpp>
#include <common/utils.hpp>
#include <format>

namespace BTCore {
UDPSocket::UDPSocket(int af) {
  m_family = af;
  m_fd = socket(af, SOCK_DGRAM, 0);

  if (m_fd < 0)
    Utils::logAndThrowFatal("UDPSocket", "Failed to create udp socket");
}

UDPSocket::UDPSocket(int af, int port) {
	m_family = af;
  m_fd = socket(af, SOCK_DGRAM, 0);

  if (m_fd < 0)
    Utils::logAndThrowFatal("UDPSocket", "Failed to create udp socket");

	bindToPort(port);
}

UDPSocket::~UDPSocket() {
  if (m_fd != -1) {
    ::close(m_fd);
  }
}

void UDPSocket::bindToPort(int port) {

  switch (m_family) {
  case (AF_INET): {
    struct sockaddr_in addr4 = {};
    addr4.sin_family = m_family;
    addr4.sin_port = htons(port);
    addr4.sin_addr.s_addr = INADDR_ANY;

    if (bind(m_fd, reinterpret_cast<struct sockaddr *>(&addr4),
             sizeof(addr4)) != 0)
      Utils::logAndThrowFatal("Binding Port", "IPv4 Port binding failed");

    break;
  }
  case (AF_INET6): {
    int on = 1;
    setsockopt(m_fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));

    struct sockaddr_in6 addr6 = {};
    addr6.sin6_family = m_family;
    addr6.sin6_port = htons(port);
    addr6.sin6_addr = in6addr_any;

    if (bind(m_fd, reinterpret_cast<struct sockaddr *>(&addr6),
             sizeof(addr6)) != 0)
      Utils::logAndThrowFatal("UDPConnector", "IPv6 Port binding failed");
    break;
  }
  default: {
    Utils::logAndThrowFatal(
        "Binding Port",
        std::format("Socket of family {} cannot be constructed", m_family));
  }
  }
}
} // namespace BTCore
