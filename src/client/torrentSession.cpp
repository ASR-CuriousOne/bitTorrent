#include <client/defs.hpp>
#include <client/torrentSession.hpp>
#include <common/utils.hpp>
#include <cstring>
#include <endian.h>
#include <netinet/in.h>

namespace BTClient {
TorrentSession::TorrentSession(NetworkEngine &networkEngine)
    : m_networkEngine(networkEngine) {}

void TorrentSession::receivePacket(BTCore::Packet packet) {
  m_incomingQueue.push(std::move(packet));
}

void TorrentSession::connectToTracker(const std::string &hostname,
                                      const std::string &port) {
  const uint64_t PROTOCOL_ID = 0x41727101980;
  const uint32_t ACTION_CONNECT = 0;

  uint32_t transactionId = m_transIdGen.generate();

	Logger::debug("connectToTracker",std::format("T id: {}",transactionId));
  m_networkEngine.registerTransaction(transactionId, weak_from_this());

  ConnectRequest req;
  req.protocolId = htobe64(PROTOCOL_ID);
  req.action = htobe32(ACTION_CONNECT);
  req.transactionId = htobe32(transactionId);

  auto messageBytes = std::as_bytes(std::span{&req, 1});

  m_networkEngine.sendPacket(hostname, port, messageBytes);

  BTCore::Packet connectionRes;
  m_incomingQueue.waitAndPop(connectionRes);

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
