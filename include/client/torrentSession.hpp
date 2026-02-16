#pragma once
#include <client/networkEngine.hpp>
#include <common/network.hpp>
#include <common/peers.hpp>
#include <common/queue.hpp>
#include <common/transIdGen.hpp>
#include <vector>

namespace BTClient {

class NetworkEngine;

class TorrentSession : public std::enable_shared_from_this<TorrentSession> {
public:
  TorrentSession(NetworkEngine &networkEngine);

  void receivePacket(BTCore::Packet packet);

  void connectToTracker(const std::string &hostname, const std::string &port);
	void announce(const std::string &hostname, const std::string &port);

private:
  NetworkEngine &m_networkEngine;
  BTCore::mutexQueue<BTCore::Packet> m_incomingQueue;
  BTCore::TransIdGen m_transIdGen;

	uint64_t m_connectionId = 0;

  std::string m_infoHash;

  std::vector<BTCore::PeerInfo> m_ipv4Peers;
  std::vector<BTCore::PeerInfoIPv6> m_ipv6Peers;
};
} // namespace BTClient
