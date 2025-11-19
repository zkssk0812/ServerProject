#pragma once
#ifndef _Socket
#include <winsock2.h>



class Socket {

public:
	Socket(SOCKET s);
	~Socket();

	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

	Socket(Socket&& other);

	Socket& operator=(Socket&& other);


	operator SOCKET() const;

private:
	SOCKET sock;
};

#endif // !_Socket
