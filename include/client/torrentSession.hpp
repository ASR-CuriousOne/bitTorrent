#pragma once
#include <common/queue.hpp>
#include <common/transIdGen.hpp>
#include <common/network.hpp>
#include <client/networkEngine.hpp>

namespace BTClient {

class NetworkEngine;

class TorrentSession : public std::enable_shared_from_this<TorrentSession> {
public:
	TorrentSession(NetworkEngine& networkEngine);

	void receivePacket(BTCore::Packet packet);

	void connectToTracker(const std::string& hostname, const std::string& port);

private:
	NetworkEngine& m_networkEngine;
	BTCore::mutexQueue<BTCore::Packet> m_incomingQueue;
	BTCore::TransIdGen m_transIdGen;

	std::string m_infoHash;

};
} // namespace BTClient
