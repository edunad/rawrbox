#include <rawrbox/network/socket.hpp>

#include <array>
#include <bit>
#include <iostream>

#ifdef _MSC_VER
	#define _WINSOCK_DEPRECATED_NO_WARNINGS

	#pragma comment(lib, "wsock32.lib")
#else

	#include <arpa/inet.h>
	#include <netdb.h>
	#include <unistd.h>

	#include <cstring>
#endif

#if !defined(SOCKET_ERROR)
	#define SOCKET_ERROR (-1)
#endif

#if !defined(SOCKET_NONE)
	#define SOCKET_NONE (0)
#endif

#if !defined(INVALID_SOCKET)
	#define INVALID_SOCKET (-1)
#endif

namespace rawrbox {
	Socket::Socket(bool ipv6) : ipv6(ipv6) {
		memset(&addr, 0, sizeof(addr));
		memset(&addr6, 0, sizeof(addr6));

#ifdef _MSC_VER
		WSAStartup(MAKEWORD(1, 1), &this->_wsda);
#endif

		this->_times.tv_sec = 0;
		this->_times.tv_usec = 0;
	}

	Socket::~Socket() {
		if (this->check())
			close();
	}

	bool Socket::check() {
		return sock > SOCKET_NONE;
	}

	bool Socket::create() {
		return this->create(IPPROTO_TCP, SOCK_STREAM);
	}

	bool Socket::create(int Protocol) {
		switch (Protocol) {
			case IPPROTO_TCP: return this->create(IPPROTO_TCP, SOCK_STREAM);
			case IPPROTO_UDP: return this->create(IPPROTO_UDP, SOCK_DGRAM);
			default: return this->create(Protocol, SOCK_RAW);
		}
	}

	bool Socket::create(int Protocol, int Type) {
		if (this->check())
			return false;

		state = SockState::skDISCONNECTED;
		sock = static_cast<int>(::socket(ipv6 ? AF_INET6 : AF_INET, Type, Protocol));
		lastCode = sock;

		return sock > SOCKET_NONE;
	}

	bool Socket::bind(unsigned short port) {
		if (!check()) return false;

		if (ipv6) {
			addr6.sin6_family = AF_INET6;
			addr6.sin6_addr = in6addr_any;
			addr6.sin6_port = htons(port);
			lastCode = ::bind(sock, std::bit_cast<struct sockaddr*>(&addr6), sizeof(addr6));
		} else {
			addr.sin_family = AF_INET;
			addr.sin_addr.s_addr = htonl(INADDR_ANY);
			addr.sin_port = htons(port);
			lastCode = ::bind(sock, std::bit_cast<struct sockaddr*>(&addr), sizeof(addr));
		}

		return this->lastCode == 0;
	}

	std::string Socket::getIpAddress() {
		std::array<char, INET6_ADDRSTRLEN> buff = {};
		inet_ntop(ipv6 ? AF_INET6 : AF_INET, ipv6 ? std::bit_cast<void*>(&addr6.sin6_addr) : std::bit_cast<void*>(&addr.sin_addr), buff.data(), sizeof(buff));
		return buff.data();
	}

	bool Socket::listen() {
		lastCode = ::listen(sock, this->_MAXCON);
		if (lastCode == SOCKET_ERROR) return false;

		state = SockState::skLISTENING;
		return true;
	}

	bool Socket::accept(Socket* socket) {
		if (!blocking && !canRead()) return false;

#ifdef _MSC_VER
		int length = ipv6 ? sizeof(socket->addr6) : sizeof(socket->addr);
#else
		auto length = static_cast<socklen_t>(ipv6 ? sizeof(socket->addr6) : sizeof(socket->addr));
#endif
		socket->sock = static_cast<int>(::accept(sock, ipv6 ? std::bit_cast<struct sockaddr*>(&socket->addr6) : std::bit_cast<struct sockaddr*>(&socket->addr), &length));

		lastCode = socket->sock;
		if (socket->sock == SOCKET_ERROR)
			return false;

		socket->state = SockState::skCONNECTED;
		return true;
	}

	void Socket::close() {
		state = SockState::skDISCONNECTED;

#ifdef _MSC_VER
		::closesocket(sock);
#else
		::shutdown(sock, SHUT_RDWR);
		::close(sock);
#endif

		memset(&addr, 0, sizeof(addr));
		memset(&addr6, 0, sizeof(addr6));
		sock = static_cast<int>(INVALID_SOCKET);
	}

	uint64_t Socket::uAddr() {
		return addr.sin_addr.s_addr;
	}

	SocketError Socket::connect(const std::string& host, unsigned short port) {
		if (!check())
			return SocketError::invalidSocket;

		struct addrinfo* result = nullptr;
		struct addrinfo hints = {};
		memset(&hints, 0, sizeof(hints));

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		auto resp = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);
		if (resp != 0)
			return SocketError::invalidHostname;

		if (result == nullptr) return SocketError::invalidHostname;

		// check if we need to switch between ipv4 and ipv6
		bool targetIsIpv6 = result->ai_family == AF_INET6;
		if (targetIsIpv6 != ipv6) {
			close();

			ipv6 = targetIsIpv6;
			create();
		}

		int connectResult = 0;
		if (ipv6) {
			memcpy(&addr6, result->ai_addr, result->ai_addrlen);

			addr6.sin6_family = AF_INET6;
			addr6.sin6_port = htons(port);
			connectResult = ::connect(sock, std::bit_cast<struct sockaddr*>(&addr6), sizeof(addr6));
		} else {
			memcpy(&addr, result->ai_addr, result->ai_addrlen);

			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			connectResult = ::connect(sock, std::bit_cast<struct sockaddr*>(&addr), sizeof(addr));
		}

		if (connectResult == SOCKET_ERROR) return SocketError::failed;

		state = SockState::skCONNECTED;
		return SocketError::success;
	}

	bool Socket::canRead() {
		FD_ZERO(&this->_scks);
		FD_SET(static_cast<unsigned>(sock), &this->_scks);

		return select(sock + 1, &this->_scks, nullptr, nullptr, &this->_times) > 0;
	}

	bool Socket::isError() {
		if (state == SockState::skERROR || sock == -1)
			return true;

		FD_ZERO(&this->_scks);
		FD_SET(static_cast<unsigned>(sock), &this->_scks);

		if (select(sock + 1, nullptr, nullptr, &this->_scks, &this->_times) >= 0)
			return false;

		state = SockState::skERROR;
		return true;
	}

	int Socket::receiveUDP(unsigned char* buffer, int size, sockaddr_in* from) {
#ifdef _MSC_VER
		int client_length = static_cast<int>(sizeof(struct sockaddr_in));
		return recvfrom(this->sock, std::bit_cast<char*>(buffer), size, 0, std::bit_cast<struct sockaddr*>(from), &client_length);
#else
		uint32_t client_length = static_cast<uint32_t>(sizeof(struct sockaddr_in));
		return recvfrom(this->sock, std::bit_cast<char*>(buffer), size, 0, std::bit_cast<struct sockaddr*>(from), &client_length);
#endif
	}

	void Socket::setTimeout(int miliseconds) {
#ifdef _MSC_VER
		DWORD timeout = miliseconds;
		setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, std::bit_cast<char*>(&timeout), sizeof(timeout));
#else
		struct timeval timeout = {};
		timeout.tv_sec = miliseconds / 1000;
		timeout.tv_usec = (miliseconds - timeout.tv_sec * 1000) * 1000;
		setsockopt(this->sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif
	}

	int Socket::receive(unsigned char* buffer, int size, int spos) {
		return ::recv(this->sock, std::bit_cast<char*>(buffer) + spos, size, 0);
	}

	bool Socket::receiveAll(unsigned char* buffer, int size, int spos) {
		int recv = 0;

		while (recv != size) {
			int ret = ::recv(this->sock, std::bit_cast<char*>(buffer) + spos + recv, size - recv, 0);
			if (ret <= 0)
				return false;

			recv += ret;
		}

		return true;
	}

	int Socket::sendUDP(const unsigned char* buffer, int size, sockaddr_in* to) {
		return ::sendto(sock, std::bit_cast<const char*>(buffer), size, 0, std::bit_cast<struct sockaddr*>(&to), sizeof(struct sockaddr_in));
	}

	int Socket::send(const unsigned char* data, int dataSize) {
		return ::send(sock, std::bit_cast<const char*>(data), dataSize, 0);
	}

	bool Socket::sendAll(const unsigned char* data, int dataSize) {
		int sent = 0;

		while (sent != dataSize) {
			int ret = ::send(sock, std::bit_cast<const char*>(data) + sent, dataSize - sent, 0);
			if (ret <= 0) return false;

			sent += ret;
		}

		return true;
	}
} // namespace rawrbox
