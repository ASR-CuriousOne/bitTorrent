#include "logger/log.hpp"
#include <cassert>
#include <client/networkEngine.hpp>
#include <client/defs.hpp>
#include <common/peers.hpp>
#include <common/utils.hpp>
#include <cstdint>
#include <cstring>
#include <endian.h>
#include <format>
#include <logger/logger.hpp>
#include <memory>
#include <string>

namespace BTClient {

NetworkEngine::NetworkEngine(int port) : m_udpCon(port) {
  m_udpCon.setOnReceive([this](BTCore::Packet pkt) {
    this->handleIncomingPacket(std::move(pkt));
  });
}

void NetworkEngine::registerTransaction(uint32_t tid,
                                 std::weak_ptr<TorrentSession> session) {
  std::lock_guard<std::mutex> lock(m_mapMutex);
  m_pendingTransactions[tid] = session;
}

void NetworkEngine::sendPacket(const std::string &host, const std::string &port,
                        const std::span<const std::byte> &data) {
  m_udpCon.sendTo(host, port, data);
}

void NetworkEngine::handleIncomingPacket(BTCore::Packet pkt) {
  if (pkt.payload.size() < 8)
    return;

  uint32_t receivedTid;
  std::memcpy(&receivedTid, pkt.payload.data() + 4, sizeof(uint32_t));
  receivedTid = ntohl(receivedTid);

	Logger::debug("handleIncomingPacket",std::format("T id: {}",receivedTid));

  std::lock_guard<std::mutex> lock(m_mapMutex);
  if (m_pendingTransactions.contains(receivedTid)) {
    std::weak_ptr<TorrentSession> weakSession =
        m_pendingTransactions[receivedTid];

    if (auto session = weakSession.lock()) {
      session->receivePacket(std::move(pkt));
    } else {
      Logger::warn("handleIncomingPacket", "Received packet for dead session. Dropping.");
    }
    m_pendingTransactions.erase(receivedTid);
  } else {
    BTCore::Utils::logAndThrowFatal(
        "handleIncomingPacket",
        "receive Id not in current transaction list.");
  }
}

} // namespace BTClient
