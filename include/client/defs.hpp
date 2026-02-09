#include <cstdint>

namespace BTClient {
struct ConnectRequest {
  uint64_t protocolId;
  uint32_t action;
  uint32_t transactionId;
} __attribute__((packed));

struct ConnectResponse {
  uint32_t action;
  uint32_t transactionId;
  uint64_t connectionId;
} __attribute__((packed));

struct AnnounceRequest {
  uint64_t connectionId;
  uint32_t action;
  uint32_t transactionId;
  uint8_t infoHash[20];
  uint8_t peerId[20];
  uint64_t downloaded;
  uint64_t left;
  uint64_t uploaded;
  uint32_t event;
  uint32_t ipAddress;
  uint32_t key;
  int32_t numWant;
  uint16_t port;
} __attribute__((packed));

struct AnnounceResponse {
  uint32_t action; 
  uint32_t transactionId;
  uint32_t interval;
  uint32_t leechers;
  uint32_t seeders;
} __attribute__((packed));

} // namespace BTClient
