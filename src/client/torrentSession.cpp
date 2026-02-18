#include <array>
#include <cassert>
#include <client/defs.hpp>
#include <client/torrentSession.hpp>
#include <common/bencoding.hpp>
#include <common/utils.hpp>
#include <cstring>
#include <endian.h>
#include <fstream>
#include <iostream>
#include <logger/logger.hpp>
#include <netinet/in.h>

namespace BTClient {
TorrentSession::TorrentSession(Logger::Logger &torrentLogger,
                               NetworkEngine &networkEngine)
    : m_torrentLogger(torrentLogger), m_networkEngine(networkEngine) {}

void TorrentSession::startTorrent(std::filesystem::path torrentFilePath) {
  std::ifstream file(torrentFilePath, std::ios::binary);
  if (!file.is_open()) {
    BTCore::Utils::logAndThrowFatal(m_torrentLogger, "InfoHash",
                                    "Failed to open file: " +
                                        torrentFilePath.string());
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string fileContent = buffer.str();

  m_fileRoot = BTCore::BDecoder::decode(fileContent);
  m_infoHash = BTCore::calculateInfoHash(m_fileRoot);
  auto tracker = BTCore::getTrackerInfo(m_fileRoot);

  assert(tracker.protocol == "udp");

  m_torrentLogger.debug("Start Torrent",
                        std::format("Hostname {} on port {} with protocol {}",
                                    tracker.hostname, tracker.port,
                                    tracker.protocol));

  connectToTracker(tracker.hostname, tracker.port);
  announce(tracker.hostname, tracker.port);
}

void TorrentSession::receivePacket(BTCore::Packet packet) {
  m_incomingQueue.push(std::move(packet));
}

void TorrentSession::connectToTracker(const std::string &hostname,
                                      const std::string &port) {
  const uint64_t PROTOCOL_ID = 0x41727101980;
  const uint32_t ACTION_CONNECT = 0;

  uint32_t transactionId = m_transIdGen.generate();

  m_torrentLogger.debug("connectToTracker",
                        std::format("T id: {}", transactionId));
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
    BTCore::Utils::logAndThrowFatal(m_torrentLogger, "connectToTracker",
                                    "Response too small");
  }

  ConnectResponse response;
  std::memcpy(&response, connectionRes.payload.data(), sizeof(ConnectResponse));

  response.action = ntohl(response.action);
  response.transactionId = ntohl(response.transactionId);
  response.connectionId = be64toh(response.connectionId);

  m_connectionId = static_cast<uint64_t>(response.connectionId);

  m_torrentLogger.info(
      "connectToTracker",
      std::format("Response received with connection id {} ", m_connectionId));
  m_torrentLogger.info(
      "connectToTracker",
      std::format("Transaction Id Orig: {} Rec: {}", transactionId,
                  static_cast<uint32_t>(response.transactionId)));
}

void TorrentSession::announce(const std::string &hostname,
                              const std::string &port) {
  const uint32_t ACTION_ANNOUNCE = 1;
  uint32_t transactionId = m_transIdGen.generate();

  m_torrentLogger.debug(
      "announce", std::format("Starting announce. T id: {}", transactionId));
  m_networkEngine.registerTransaction(transactionId, weak_from_this());

  AnnounceRequest req;
  req.connectionId = htobe64(m_connectionId);
  req.action = htobe32(ACTION_ANNOUNCE);
  req.transactionId = htobe32(transactionId);

  std::memcpy(req.infoHash, m_infoHash.data(), 20);

  std::string peerId = "-BT1000-" + std::to_string(m_transIdGen.generate());
  peerId.resize(20, '0');
  std::memcpy(req.peerId, peerId.data(), 20);

  req.downloaded = htobe64(0);
  req.left = htobe64(BTCore::getTorrentSize(m_fileRoot));
  req.uploaded = htobe64(0);
  req.event = htobe32(0);
  req.ipAddress = 0;
  req.key = htobe32(m_transIdGen.generate());
  req.numWant = htobe32(-1);
  req.port = htobe16(m_networkEngine.getPort());

  auto messageBytes = std::as_bytes(std::span{&req, 1});
  m_networkEngine.sendPacket(hostname, port, messageBytes);

  BTCore::Packet announceResPkt;
  m_incomingQueue.waitAndPop(announceResPkt);

  if (announceResPkt.payload.size() < sizeof(AnnounceResponse)) {
    BTCore::Utils::logAndThrowFatal(m_torrentLogger, "announce",
                                    "Response too small to be valid");
  }

  AnnounceResponse response;
  std::memcpy(&response, announceResPkt.payload.data(),
              sizeof(AnnounceResponse));

  response.action = ntohl(response.action);
  response.transactionId = ntohl(response.transactionId);
  response.interval = ntohl(response.interval);
  response.leechers = ntohl(response.leechers);
  response.seeders = ntohl(response.seeders);

  if (response.action == 3) {
    if (announceResPkt.payload.size() > 8) {
      std::string errorMsg(announceResPkt.payload.begin() + 8,
                           announceResPkt.payload.end());
      m_torrentLogger.error("announce", "Tracker returned error: " + errorMsg);
    } else {
      m_torrentLogger.error("announce", "Tracker returned error (no message)");
    }
    return;
  }

  if (response.transactionId != transactionId) {
    m_torrentLogger.error("announce", "Transaction ID mismatch in response");
    return;
  }
  if (response.action != ACTION_ANNOUNCE) {
    m_torrentLogger.error("announce",
                          std::format("Action mismatch in response {}",
                                      static_cast<uint32_t>(response.action)));
    return;
  }

  uint32_t interval = ntohl(response.interval);
  uint32_t leechers = ntohl(response.leechers);
  uint32_t seeders = ntohl(response.seeders);

  m_torrentLogger.info(
      "announce",
      std::format("Tracker Response - Interval: {}s, Leechers: {}, Seeders: {}",
                  interval, leechers, seeders));

  size_t headerSize = sizeof(AnnounceResponse);
  if (announceResPkt.payload.size() > headerSize) {
    size_t dataLen = announceResPkt.payload.size() - headerSize;
    size_t numPeers = dataLen / 6;

    const uint8_t *peerDataPtr = reinterpret_cast<const uint8_t *>(
        announceResPkt.payload.data() + headerSize);

    for (size_t i = 0; i < numPeers; ++i) {
      BTCore::PeerInfo peer;
      std::memcpy(&peer, peerDataPtr + (i * 6), 6);

      m_ipv4Peers.push_back(peer);
    }
    m_torrentLogger.info(
        "announce",
        std::format("Successfully parsed {} peers.", m_ipv4Peers.size()));
  }
}

} // namespace BTClient
