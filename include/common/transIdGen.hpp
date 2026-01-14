#pragma once
#include <random>

namespace BTCore {
class TransIdGen {
private:
  std::mt19937 rng;
  std::uniform_int_distribution<uint32_t> dist;

public:
	TransIdGen();
	uint32_t generate();
	uint32_t generateNetworkOrder();
};

} // namespace BTCore
