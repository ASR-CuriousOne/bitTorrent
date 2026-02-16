#pragma once
#include <array>
#include <cstdint>
#include <string>

namespace BTCore {

class SHA1 {
public:
  SHA1();
  void update(const std::string &s);
  void update(const void *data, size_t len);

  void finalize();

  std::array<uint32_t, 5> getDigest() const;

private:
  uint32_t m_digest[5];
  uint8_t m_buffer[64];
  size_t m_transforms;

  void transform(uint8_t *block);
};

} // namespace BTCore
