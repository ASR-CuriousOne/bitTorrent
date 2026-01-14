#include <cassert>
#include <client/bitClient.hpp>
#include <common/utils.hpp>
#include <cstdint>
#include <cstring>
#include <endian.h>
#include <logger/logger.hpp>
#include <string>

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

Client::Client(int port) : m_port(port), m_udpCon(port) {
  m_udpCon.setOnReceive(
      [this](BTCore::Packet pkt) { m_responseQueue.push(pkt); });
}

void Client::connectToTracker(const std::string &hostname,
                              const std::string &port) {
  const uint64_t PROTOCOL_ID = 0x41727101980;
  const uint32_t ACTION_CONNECT = 0;

  uint32_t transactionId = m_transIdGen.generateNetworkOrder();
  ConnectRequest req;
  req.protocolId = htobe64(PROTOCOL_ID);
  req.action = htobe32(ACTION_CONNECT);
  req.transactionId = htobe32(transactionId);

  auto messageBytes = std::as_bytes(std::span{&req, 1});

  m_udpCon.sendTo(hostname, port, messageBytes);

  BTCore::Packet connectionRes;
  m_responseQueue.waitAndPop(connectionRes);

  if (connectionRes.payload.size() < sizeof(ConnectResponse)) {
    BTCore::Utils::logAndThrowFatal("connectToTracker", "Response too small");
  }

  ConnectResponse response;
  std::memcpy(&response, connectionRes.payload.data(), sizeof(ConnectResponse));

  response.action = ntohl(response.action);
  response.transactionId = ntohl(response.transactionId);
  response.connectionId = be32toh(response.connectionId);

  Logger::info("connectToTracker",
               std::format("Response received with connection id {} ",
                           static_cast<uint64_t>(response.connectionId)));
  Logger::info("connectToTracker",
               std::format("Transaction Id Orig: {} Rec: {}", transactionId,
                           static_cast<uint32_t>(response.transactionId)));
}

} // namespace BTClient
