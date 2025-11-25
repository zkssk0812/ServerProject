#include "GameServer.h"

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

	// [수정됨] 김윤기: 서버 객체 생성 및 실행 코드 추가
	GameServer server(9000);
	server.ready();

	return 0;
}