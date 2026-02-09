#pragma once
#include <client/torrentSession.hpp>
#include <common/network.hpp>
#include <common/queue.hpp>
#include <common/transIdGen.hpp>
#include <map>
#include <string>
#include <memory>

namespace BTClient {

class TorrentSession;

class NetworkEngine {
private:
  const int m_port = 6881;
  BTCore::UDPConnector m_udpCon;
  std::map<uint32_t, std::weak_ptr<TorrentSession>> m_pendingTransactions;
	std::mutex m_mapMutex;

	void handleIncomingPacket(BTCore::Packet pkt);

public:
  NetworkEngine(int port = 6881);

  void run();

  void sendPacket(const std::string& host, const std::string& port, 
                    const std::span<const std::byte>& data);
	void registerTransaction(uint32_t tid, std::weak_ptr<TorrentSession> session);
};
} // namespace BTClient
