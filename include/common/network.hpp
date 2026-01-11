#pragma once
#include <atomic>
#include <cstddef>
#include <span>
#include <string>
#include <thread>
#include <vector>

namespace BTCore {

class UDPConnector {
private:
	std::atomic<bool> m_isRunning = false;
	std::jthread m_recieverThread;

	const int m_port = 6881;

	int m_sockv4 = -1;
	int m_sockv6 = -1;

	void initSockets();

public:
	UDPConnector(int port);

	~UDPConnector();

	int sendToTracker(const std::string& host,const std::string& port, std::span<std::byte> bytes);

	void recieve();

	void handleRead(int fd, std::vector<char>& buffer);

};
} // namespace BTCore
