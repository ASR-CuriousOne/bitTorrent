#pragma once
#include <client/torrentSession.hpp>
#include <common/network.hpp>
#include <common/queue.hpp>
#include <common/transIdGen.hpp>
#include <logger/logger.hpp>
#include <map>
#include <memory>
#include <string>

namespace BTClient {

class TorrentSession;

class NetworkEngine {
private:
  Logger::Logger &m_networkLogger;
  const int m_port = 6881;
  BTCore::UDPConnector m_udpCon;
  std::map<uint32_t, std::weak_ptr<TorrentSession>> m_pendingTransactions;
  std::mutex m_mapMutex;

  void handleIncomingPacket(BTCore::Packet pkt);

public:
  NetworkEngine(Logger::Logger &networkLogger, int port = 6881);

  void run();

  void sendPacket(const std::string &host, const std::string &port,
                  const std::span<const std::byte> &data);
  void registerTransaction(uint32_t tid, std::weak_ptr<TorrentSession> session);

  int getPort() const { return m_port; }
};
} // namespace BTClient
