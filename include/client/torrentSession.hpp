#pragma once
#include <array>
#include <client/networkEngine.hpp>
#include <common/bencoding.hpp>
#include <common/network.hpp>
#include <common/peers.hpp>
#include <common/queue.hpp>
#include <common/transIdGen.hpp>
#include <filesystem>
#include <logger/logger.hpp>
#include <vector>

namespace BTClient {

class NetworkEngine;

class TorrentSession : public std::enable_shared_from_this<TorrentSession> {
public:
  BTCore::BNode m_fileRoot;

  TorrentSession(Logger::Logger &torrentLogger, NetworkEngine &networkEngine);

  void startTorrent(std::filesystem::path torrentFilePath);

  void receivePacket(BTCore::Packet packet);

  void connectToTracker(const std::string &hostname, const std::string &port);
  void announce(const std::string &hostname, const std::string &port);

private:
  Logger::Logger &m_torrentLogger;
  NetworkEngine &m_networkEngine;
  BTCore::mutexQueue<BTCore::Packet> m_incomingQueue;
  BTCore::TransIdGen m_transIdGen;

  uint64_t m_connectionId = 0;

  std::array<uint32_t, 5> m_infoHash;

  std::vector<BTCore::PeerInfo> m_ipv4Peers;
  std::vector<BTCore::PeerInfoIPv6> m_ipv6Peers;
};
} // namespace BTClient
