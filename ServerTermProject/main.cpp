#include "GameServer.h"
#define BUFSIZE 512

class WSAStart {
public:
	WSAStart() {
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			exit(1);
	}
	~WSAStart() {
		WSACleanup();
	}
private:
	WSADATA wsa;
};

int main() {
	WSAStart wsa;
	GameServer gs{ 9000 } ;
	gs.ready();
}