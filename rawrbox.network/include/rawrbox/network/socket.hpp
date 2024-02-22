#pragma once

#include <fcntl.h>

#include <cctype>
#include <cinttypes>
#include <sstream>
#include <string>
#include <vector>

#ifdef _MSC_VER
	#define _WINSOCKAPI_
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <netinet/in.h>
#endif

namespace rawrbox {
	enum class SocketError {
		success,
		invalidSocket,
		invalidHostname,
		failed
	};

	enum class SockState {
		skDISCONNECTED = 0,
		skUNDEF1, // Not implemented
		skLISTENING,
		skUNDEF3, // Not implemented
		skUNDEF4, // Not implemented
		skUNDEF5, // Not implemented
		skUNDEF6, // Not implemented
		skCONNECTED,
		skERROR
	};

	class Socket {
	private:
#ifdef _MSC_VER
		WSADATA _wsda = {};
#endif
		int _MAXCON = 64;

		fd_set _scks = {};
		timeval _times = {};

		uint8_t _totaldata = 0;

		[[nodiscard]] bool check() const;

	public:
		bool blocking = true;
		bool ipv6 = false;

		sockaddr_in addr = {};
		sockaddr_in fromAddr = {};
		sockaddr_in6 addr6 = {};
		sockaddr_in6 fromAddr6 = {};
		unsigned long fromAddr_len = 0;

		SockState state = SockState::skDISCONNECTED;
		int sock = -1;

		int lastCode = 0;

		Socket(bool ipv6 = false);
		Socket(const Socket&) = default;
		Socket(Socket&&) = delete;
		Socket& operator=(const Socket&) = default;
		Socket& operator=(Socket&&) = delete;
		~Socket();

		bool create();
		bool create(int Protocol);
		bool create(int Protocol, int Type);
		bool bind(uint16_t port);
		bool listen();
		bool accept(Socket* socket);
		[[nodiscard]] rawrbox::SocketError connect(const std::string& host, uint16_t port);
		void close();

		[[nodiscard]] uint64_t uAddr() const;
		bool isError();

		std::string getIpAddress();

		bool canRead();

		int receive(uint8_t* buffer, int size, int spos = 0) const;
		bool receiveAll(uint8_t* buffer, int size, int spos = 0) const;
		int send(const uint8_t* data, int dataSize) const;
		bool sendAll(const uint8_t* data, int dataSize) const;
		int sendUDP(const uint8_t* buffer, int size, sockaddr_in* to) const;
		int receiveUDP(uint8_t* buffer, int size, sockaddr_in* from) const;
		void setTimeout(int miliseconds) const;
	};
} // namespace rawrbox
