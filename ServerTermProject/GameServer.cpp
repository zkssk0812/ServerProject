#include "GameServer.h"
#include <stdio.h>



GameServer::GameServer(int port) : listen_sock(socket(AF_INET, SOCK_STREAM, 0)), Port(port) {

	if (listen_sock == INVALID_SOCKET)
		err_quit("socket()");

	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(Port);

	retval = bind(listen_sock, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");
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
	while (true) {
		Socket client_sock{ accept(listen_sock, reinterpret_cast<sockaddr*>(&clientaddr), &addrlen) };
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			continue;
		}
	

		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d", addr, ntohs(clientaddr.sin_port));

		

		char buf[4]{};
		memcpy(buf, &PlayerCount, sizeof(PlayerCount));

		retval = send(client_sock, buf, sizeof(PlayerCount), 0);

		std::shared_ptr<Socket> clientSocketPtr = std::make_shared<Socket>(std::move(client_sock));

		ThreadArgs* args = new ThreadArgs{ this, clientSocketPtr };

		EnterCriticalSection(&m_clientListCS);
		m_clientList[PlayerCount] = clientSocketPtr;
		LeaveCriticalSection(&m_clientListCS);

		++PlayerCount;

		hThread[PlayerCount] = CreateThread(NULL, 0, ClientThread, args, 0, NULL);

		

		if (PlayerCount >= 2) break;
		
	}

	this->run();

}

DWORD WINAPI GameServer::ClientThread(LPVOID arg) {
	ThreadArgs* args = static_cast<ThreadArgs*>(arg);
	
	while (true) {
		int taskType;
		char buf[DATASIZE]{};
		recv(*args->sock, buf, sizeof(buf), 0);
		memcpy(&taskType, buf, sizeof(int));
		BaseTask* BT = new BaseTask;
		BT->clientSocket = args->sock;
		BT->taskType = taskType;
		memcpy(BT->data, buf, sizeof(buf));

		args->server->threadQueue.Push(BT);
	}
	return 0;
}


void GameServer::run() {
	

	hThread[0] = CreateThread(NULL, 0, GameLogic, this, 0, NULL);
}

DWORD WINAPI GameServer::GameLogic(LPVOID arg) {

	while (true) {
		GameServer* server = static_cast<GameServer*>(arg);
		BaseTask* bs = server->threadQueue.Pop();
		
		if (bs->taskType == 100) {
			
			server->HandleCardRequest(bs);
		}
		if (bs->taskType == 101) {
			server->HandleCollisionRequest(bs);
		}
	}
	
	return 0;

}

void GameServer::HandleCardRequest(BaseTask* arg) {

	for (int i = 0; i < 2; ++i) {
		EnterCriticalSection(&m_clientListCS);
		send(*m_clientList[i], arg->data, sizeof(arg->data), 0);
		LeaveCriticalSection(&m_clientListCS);
	}

	delete arg;
}

void GameServer::HandleCollisionRequest(BaseTask* arg) {
	for (int i = 0; i < 2; ++i) {
		EnterCriticalSection(&m_clientListCS);
		send(*m_clientList[i], arg->data, sizeof(arg->data), 0);
		LeaveCriticalSection(&m_clientListCS);
	}
	delete arg;
}


