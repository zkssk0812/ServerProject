//GameServer.cpp
#include "GameServer.h"
#include <stdio.h>



GameServer::GameServer(int port) : listen_sock(socket(AF_INET, SOCK_STREAM, 0)), Port(port) {

	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(Port);

	retval = bind(listen_sock, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// [추가] CS 초기화가 생성자에서 필요.
	InitializeCriticalSection(&m_clientListCS);
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	MessageBoxA(NULL, (const char*)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void GameServer::ready() {
	static int PlayerCount = 0;
	sockaddr_in clientaddr;
	int addrlen = sizeof(clientaddr);

	printf("[TCP 서버] 접속 대기 중...\n");

	while (true) {
		Socket client_sock{ accept(listen_sock, reinterpret_cast<sockaddr*>(&clientaddr), &addrlen) };
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			continue;
		}

		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[TCP 서버] 클라이언트 접속: IP=%s, Port=%d\n", addr, ntohs(clientaddr.sin_port));

		// 플레이어 번호 전송 (0 또는 1)
		retval = send(client_sock, (char*)&PlayerCount, sizeof(int), 0);

		std::shared_ptr<Socket> clientSocketPtr = std::make_shared<Socket>(std::move(client_sock));
		ThreadArgs* args = new ThreadArgs{ this, clientSocketPtr };

		EnterCriticalSection(&m_clientListCS);
		m_clientList[PlayerCount] = clientSocketPtr;
		LeaveCriticalSection(&m_clientListCS);

		// 클라이언트별 수신 스레드 시작
		hThread[PlayerCount + 1] = CreateThread(NULL, 0, ClientThread, args, 0, NULL); // 인덱스 조정 (+1은 로직스레드 공간 확보)

		printf("[서버] 플레이어 %d 접속 완료\n", PlayerCount);
		++PlayerCount;

		if (PlayerCount >= 2) {
			printf("[서버] 모든 플레이어 접속 완료. 게임 로직 시작.\n");
			break;
		}
	}
	this->run();
}

DWORD WINAPI GameServer::ClientThread(LPVOID arg) {
	ThreadArgs* args = static_cast<ThreadArgs*>(arg);
	GameServer* server = args->server;

	Socket& clientSock = *(args->sock);

	printf("[서버] 클라이언트 스레드 시작\n");

	char buf[512];

	while (true) {

		int retval = recv(clientSock, buf, 512, 0);


		if (retval == SOCKET_ERROR || retval == 0) {
			printf("[서버] 클라이언트 접속 종료\n");
			break;
		}

		// [프로토콜 처리]
		// 앞 4바이트: Protocol ID (100 or 101)
		if (retval >= sizeof(int)) {
			int protocol;
			memcpy(&protocol, buf, sizeof(int));

			BaseTask* bs = new BaseTask();
			bs->clientSocket = args->sock; // 누가 보냈는지 식별
			bs->taskType = protocol;

			int dataLen = retval - sizeof(int);
			if (dataLen > 0) {
				if (dataLen > DATASIZE) dataLen = DATASIZE;
				memcpy(bs->data, buf + sizeof(int), dataLen);
			}
			// 스레드 큐에 Push (생산자)
			server->threadQueue.Push(bs);
		}
	}
	delete args;
	return 0;
}


void GameServer::run() {
	// 로직 처리 스레드 (소비자)
	hThread[0] = CreateThread(NULL, 0, GameLogic, this, 0, NULL);
	WaitForMultipleObjects(3, hThread, TRUE, INFINITE); // 메인 스레드가 종료되지 않게 대기
}

DWORD WINAPI GameServer::GameLogic(LPVOID arg) {
	GameServer* server = static_cast<GameServer*>(arg);

	while (true) {
		// 큐에서 작업 꺼내기 (소비자)
		BaseTask* bs = server->threadQueue.Pop();

		if (bs->taskType == 100) {
			// [수정됨] 김윤기: bs->data가 아니라 bs 구조체 포인터를 넘겨야 함
			server->HandleCardRequest(bs);
			delete bs;
		}
		else if (bs->taskType == 101) {
			server->HandleCollisionRequest(bs);
			delete bs;
		}
	}
	return 0;
}

void GameServer::HandleCardRequest(BaseTask* arg) {
	int senderNum = -1;

	// 1. 누가 보냈는지 찾기
	EnterCriticalSection(&m_clientListCS);
	for (int i = 0; i < 2; ++i) {
		if (arg->clientSocket == m_clientList[i]) {
			senderNum = i;
			break;
		}
	}

	// 2. 다른 플레이어에게(혹은 모두에게) 데이터 전송
	// 카드 사용 정보: [프로토콜(100)] + [누가썼는지(int)] + [카드데이터(나머지)]
	// 여기서는 간단히 받은 데이터를 그대로 브로드캐스팅하는 예시

	// 클라이언트는 [int:누가] + [data] 를 받을 준비가 되어 있어야함ㅁ.
	// LLD에 따르면 서버는 동기화 정보를 줘야 함.

	char packet[sizeof(int) + DATASIZE];
	int protocol = 100; // Client의 taskType과 맞춰줌

	// 패킷 조립 (예시: 프로토콜헤더 + 보낸사람ID + 원본데이터)
	// 실제 클라이언트 Recv 처리에 맞춰야 하지만, 일단 받은 데이터를 그대로 전달

	for (int i = 0; i < 2; ++i) {
		// 단순 에코(Echo) or 브로드캐스트
		// 보낸 사람의 ID를 맨 앞에 붙여서 보내는 것이 일반적입니다.
		send(*m_clientList[i], (char*)&protocol, sizeof(int), 0);
		send(*m_clientList[i], (char*)&senderNum, sizeof(int), 0); // 누가 썼는지
		send(*m_clientList[i], arg->data, sizeof(arg->data), 0);   // 무슨 카드인지
	}
	LeaveCriticalSection(&m_clientListCS);
}

void GameServer::HandleCollisionRequest(BaseTask* arg) {
	int senderNum = -1;

	EnterCriticalSection(&m_clientListCS);
	for (int i = 0; i < 2; ++i) {
		if (arg->clientSocket == m_clientList[i]) {
			senderNum = i;
			break;
		}
	}

	int protocol = 101;

	// 모든 클라이언트에게 충돌 사실 알림
	for (int i = 0; i < 2; ++i) {
		send(*m_clientList[i], (char*)&protocol, sizeof(int), 0);
		send(*m_clientList[i], (char*)&senderNum, sizeof(int), 0); // 누가 충돌했는지
		// 필요한 경우 arg->data 추가 전송
	}
	LeaveCriticalSection(&m_clientListCS);
}
