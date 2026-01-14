#include <common/transIdGen.hpp>
#include <limits>
#include <arpa/inet.h>
#include <cstdint>

namespace BTCore {
TransIdGen::TransIdGen() {
  std::random_device rd;

  rng.seed(rd());

  dist = std::uniform_int_distribution<uint32_t>(
      0, std::numeric_limits<uint32_t>::max());
}

uint32_t TransIdGen::generate() { return dist(rng); }

uint32_t TransIdGen::generateNetworkOrder() { return htonl(dist(rng)); }
} // namespace BTCore
