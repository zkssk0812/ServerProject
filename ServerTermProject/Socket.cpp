#include "Socket.h"

Socket::Socket(SOCKET s) : sock(s) {}
Socket::~Socket() {
	if (sock != INVALID_SOCKET) {
		closesocket(sock);
	}
}


Socket::Socket(Socket&& other) : sock(other.sock) {
	other.sock = INVALID_SOCKET;
}

Socket& Socket::operator=(Socket&& other) {
	if (this == &other) {
		return *this;
	}

	if (sock != INVALID_SOCKET) closesocket(sock);

	sock = other.sock;
	other.sock = INVALID_SOCKET;

	return *this;
}


Socket::operator SOCKET() const {
	return sock;
}