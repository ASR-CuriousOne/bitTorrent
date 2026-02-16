#include <common/sha1.hpp>
#include <cstring>

namespace BTCore {

// Rotate Left Helper
static uint32_t rol(uint32_t value, size_t bits) {
  return (value << bits) | (value >> (32 - bits));
}

// Byte-swap Helper (Big Endian conversion for input block)
static uint32_t blk(const uint8_t *block, size_t i) {
  return (block[i * 4] << 24) | (block[i * 4 + 1] << 16) |
         (block[i * 4 + 2] << 8) | (block[i * 4 + 3]);
}

SHA1::SHA1() : m_transforms(0) {
  m_digest[0] = 0x67452301;
  m_digest[1] = 0xEFCDAB89;
  m_digest[2] = 0x98BADCFE;
  m_digest[3] = 0x10325476;
  m_digest[4] = 0xC3D2E1F0;
}

void SHA1::update(const std::string &s) { update(s.data(), s.size()); }

void SHA1::update(const void *data, size_t len) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);

  for (size_t i = 0; i < len; i++) {
    m_buffer[m_transforms % 64] = ptr[i];
    m_transforms++;
    if (m_transforms % 64 == 0) {
      transform(m_buffer);
    }
  }
}

void SHA1::finalize() {
  uint64_t total_bits = m_transforms * 8;

  // Append 0x80
  m_buffer[m_transforms % 64] = 0x80;
  m_transforms++;

  // Pad with zeros until we have space for the 64-bit length
  if (m_transforms % 64 > 56) {
    while (m_transforms % 64 != 0) {
      m_buffer[m_transforms % 64] = 0x00;
      m_transforms++;
    }
    transform(m_buffer);
  }

  while (m_transforms % 64 < 56) {
    m_buffer[m_transforms % 64] = 0x00;
    m_transforms++;
  }

  // Append total length as big endian
  for (int i = 0; i < 8; i++) {
    m_buffer[63 - i] = (total_bits >> (i * 8)) & 0xFF;
  }
  transform(m_buffer);
}

std::array<uint32_t, 5> SHA1::getDigest() const {
  return {m_digest[0], m_digest[1], m_digest[2], m_digest[3], m_digest[4]};
}

void SHA1::transform(uint8_t *block) {
  uint32_t a = m_digest[0];
  uint32_t b = m_digest[1];
  uint32_t c = m_digest[2];
  uint32_t d = m_digest[3];
  uint32_t e = m_digest[4];

  uint32_t w[80];

  for (size_t i = 0; i < 16; i++) {
    w[i] = blk(block, i);
  }
  for (size_t i = 16; i < 80; i++) {
    w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
  }

  auto f1 = [](uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) | (~x & z);
  };
  auto f2 = [](uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; };
  auto f3 = [](uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) | (x & z) | (y & z);
  };
  auto f4 = [](uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; };

  for (size_t i = 0; i < 20; i++) {
    uint32_t t = rol(a, 5) + f1(b, c, d) + e + w[i] + 0x5A827999;
    e = d;
    d = c;
    c = rol(b, 30);
    b = a;
    a = t;
  }
  for (size_t i = 20; i < 40; i++) {
    uint32_t t = rol(a, 5) + f2(b, c, d) + e + w[i] + 0x6ED9EBA1;
    e = d;
    d = c;
    c = rol(b, 30);
    b = a;
    a = t;
  }
  for (size_t i = 40; i < 60; i++) {
    uint32_t t = rol(a, 5) + f3(b, c, d) + e + w[i] + 0x8F1BBCDC;
    e = d;
    d = c;
    c = rol(b, 30);
    b = a;
    a = t;
  }
  for (size_t i = 60; i < 80; i++) {
    uint32_t t = rol(a, 5) + f4(b, c, d) + e + w[i] + 0xCA62C1D6;
    e = d;
    d = c;
    c = rol(b, 30);
    b = a;
    a = t;
  }

  m_digest[0] += a;
  m_digest[1] += b;
  m_digest[2] += c;
  m_digest[3] += d;
  m_digest[4] += e;
}

} // namespace BTCore
