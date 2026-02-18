#pragma once
#include <logger/logger.hpp>
#include <stdexcept>

namespace BTCore::Utils {
[[noreturn]] void inline logAndThrowFatal(Logger::Logger &logger, const std::string &origin,
                                          const std::string &message) {
  logger.fatal(origin, message);
  throw std::runtime_error(message);
}

inline std::string toHex(const std::array<uint32_t, 5>& digest) {
    std::string hex;
    hex.reserve(40); // Pre-allocate for performance
    
    for (const auto& word : digest) {
        hex += std::format("{:08x}", word);
    }
    return hex;
}

} // namespace BTCore::Utils
