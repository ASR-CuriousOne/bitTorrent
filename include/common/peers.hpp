#include <cstdint>

namespace BTCore {
struct PeerInfo {
  uint32_t ip;
  uint16_t port;
} __attribute__((packed));

struct PeerInfoIPv6 {
  uint8_t ip[16]; 
  uint16_t port; 
} __attribute__((packed));
} // namespace BTCore
