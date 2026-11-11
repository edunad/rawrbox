#include <rawrbox/network/socket.hpp>

#include <tracy/Tracy.hpp>

#include <array>
#include <bit>

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
		memset(&this->addr, 0, sizeof(this->addr));
		memset(&this->addr6, 0, sizeof(this->addr6));

#ifdef _MSC_VER
		WSAStartup(MAKEWORD(1, 1), &this->_wsda);
#endif

		this->_times.tv_sec = 0;
		this->_times.tv_usec = 0;

		this->create();
	}

	Socket::~Socket() {
		if (this->check())
			this->close();
	}

	bool Socket::check() const {
		return this->sock > SOCKET_NONE;
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

		this->state = SockState::skDISCONNECTED;
		this->sock = static_cast<int>(::socket(this->ipv6 ? AF_INET6 : AF_INET, Type, Protocol));
		this->lastCode = this->sock;

		return this->sock > SOCKET_NONE;
	}

	bool Socket::bind(uint16_t port) {
		if (!this->check()) return false;

		if (this->ipv6) {
			this->addr6.sin6_family = AF_INET6;
			this->addr6.sin6_addr = in6addr_any;
			this->addr6.sin6_port = htons(port);
			this->lastCode = ::bind(this->sock, std::bit_cast<struct sockaddr*>(&this->addr6), sizeof(this->addr6));
		} else {
			this->addr.sin_family = AF_INET;
			this->addr.sin_addr.s_addr = htonl(INADDR_ANY);
			this->addr.sin_port = htons(port);
			this->lastCode = ::bind(this->sock, std::bit_cast<struct sockaddr*>(&this->addr), sizeof(this->addr));
		}

		return this->lastCode == 0;
	}

	std::string Socket::getIpAddress() {
		std::array<char, INET6_ADDRSTRLEN> buff = {};
		inet_ntop(this->ipv6 ? AF_INET6 : AF_INET, this->ipv6 ? std::bit_cast<void*>(&this->addr6.sin6_addr) : std::bit_cast<void*>(&this->addr.sin_addr), buff.data(), sizeof(buff));
		return buff.data();
	}

	bool Socket::listen() {
		this->lastCode = ::listen(this->sock, this->_MAXCON);
		if (this->lastCode == SOCKET_ERROR) return false;

		this->state = SockState::skLISTENING;
		return true;
	}

	bool Socket::accept(Socket* socket) {
		if (!this->blocking && !this->canRead()) return false;
		ZoneScopedN("Socket::accept");

#ifdef _MSC_VER
		int length = this->ipv6 ? sizeof(socket->addr6) : sizeof(socket->addr);
#else
		auto length = static_cast<socklen_t>(ipv6 ? sizeof(socket->addr6) : sizeof(socket->addr));
#endif
		socket->sock = static_cast<int>(::accept(this->sock, this->ipv6 ? std::bit_cast<struct sockaddr*>(&socket->addr6) : std::bit_cast<struct sockaddr*>(&socket->addr), &length));

		this->lastCode = socket->sock;
		if (socket->sock == SOCKET_ERROR)
			return false;

		socket->state = SockState::skCONNECTED;
		return true;
	}

	void Socket::close() {
		this->state = SockState::skDISCONNECTED;

#ifdef _MSC_VER
		::closesocket(this->sock);
#else
		::shutdown(sock, SHUT_RDWR);
		::close(sock);
#endif

		memset(&this->addr, 0, sizeof(this->addr));
		memset(&this->addr6, 0, sizeof(this->addr6));
		this->sock = static_cast<int>(INVALID_SOCKET);
	}

	uint64_t Socket::uAddr() const {
		return this->addr.sin_addr.s_addr;
	}

	SocketError Socket::connect(const std::string& host, uint16_t port) {
		ZoneScopedN("Socket::connect");
		if (!this->check())
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
		if (targetIsIpv6 != this->ipv6) {
			this->close();

			this->ipv6 = targetIsIpv6;
			this->create();
		}

		int connectResult = 0;
		if (this->ipv6) {
			if (result->ai_addrlen <= sizeof(this->addr6)) {
				std::memcpy(&this->addr6, result->ai_addr, result->ai_addrlen);
			} else {
				throw std::runtime_error("Not enough space for ipv6 address");
			}

			this->addr6.sin6_family = AF_INET6;
			this->addr6.sin6_port = htons(port);
			connectResult = ::connect(this->sock, std::bit_cast<struct sockaddr*>(&this->addr6), sizeof(this->addr6));
		} else {
			if (result->ai_addrlen <= sizeof(this->addr)) {
				std::memcpy(&this->addr, result->ai_addr, result->ai_addrlen);
			} else {
				throw std::runtime_error("Not enough space for ipv4 address");
			}

			this->addr.sin_family = AF_INET;
			this->addr.sin_port = htons(port);
			connectResult = ::connect(this->sock, std::bit_cast<struct sockaddr*>(&this->addr), sizeof(this->addr));
		}

		if (connectResult == SOCKET_ERROR) return SocketError::failed;

		this->state = SockState::skCONNECTED;
		return SocketError::success;
	}

	bool Socket::canRead() {
		FD_ZERO(&this->_scks);
		FD_SET(static_cast<unsigned>(this->sock), &this->_scks);

		return select(this->sock + 1, &this->_scks, nullptr, nullptr, &this->_times) > 0;
	}

	bool Socket::isError() {
		if (this->state == SockState::skERROR || this->sock == -1)
			return true;

		FD_ZERO(&this->_scks);
		FD_SET(static_cast<unsigned>(this->sock), &this->_scks);

		if (select(this->sock + 1, nullptr, nullptr, &this->_scks, &this->_times) >= 0)
			return false;

		this->state = SockState::skERROR;
		return true;
	}

	int Socket::receiveUDP(uint8_t* buffer, int size, sockaddr_in* from) const {
#ifdef _MSC_VER
		int client_length = static_cast<int>(sizeof(struct sockaddr_in));
		return recvfrom(this->sock, std::bit_cast<char*>(buffer), size, 0, std::bit_cast<struct sockaddr*>(from), &client_length);
#else
		auto client_length = static_cast<uint32_t>(sizeof(struct sockaddr_in));
		return recvfrom(this->sock, std::bit_cast<char*>(buffer), size, 0, std::bit_cast<struct sockaddr*>(from), &client_length);
#endif
	}

	void Socket::setTimeout(int miliseconds) const {
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

	int Socket::receive(uint8_t* buffer, int size, int spos) const {
		return ::recv(this->sock, std::bit_cast<char*>(buffer) + spos, size, 0);
	}

	bool Socket::receiveAll(uint8_t* buffer, int size, int spos) const {
		ZoneScopedN("Socket::receiveAll");
		int recv = 0;

		while (recv != size) {
			int ret = ::recv(this->sock, std::bit_cast<char*>(buffer) + spos + recv, size - recv, 0);
			if (ret <= 0)
				return false;

			recv += ret;
		}

		return true;
	}

	int Socket::sendUDP(const uint8_t* buffer, int size, sockaddr_in* to) const {
		return ::sendto(this->sock, std::bit_cast<const char*>(buffer), size, 0, std::bit_cast<struct sockaddr*>(&to), sizeof(struct sockaddr_in));
	}

	int Socket::send(const uint8_t* data, int dataSize) const {
		return ::send(this->sock, std::bit_cast<const char*>(data), dataSize, 0);
	}

	bool Socket::sendAll(const uint8_t* data, int dataSize) const {
		ZoneScopedN("Socket::sendAll");
		int sent = 0;

		while (sent != dataSize) {
			int ret = ::send(this->sock, std::bit_cast<const char*>(data) + sent, dataSize - sent, 0);
			if (ret <= 0) return false;

			sent += ret;
		}

		return true;
	}
} // namespace rawrbox
