#pragma once
#include <logger/logger.hpp>
#include <stdexcept>

namespace BTCore::Utils{
[[noreturn]] void inline logAndThrowFatal(const std::string &origin,
                                          const std::string &message) {
  Logger::fatal(origin, message);
  throw std::runtime_error(message);
}
}

