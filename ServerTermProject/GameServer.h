#pragma once
#ifndef _GameServer


#include <ws2tcpip.h>
#include "ThreadSafeQueue.h"
#include "Socket.h"

#pragma comment(lib, "ws2_32")

#define THREADNUM 3
#define LOGICTHREAD 0
#define CLIENTTHREAD1 1
#define CLIENTTHREAD2 2
#define CLIENTTHREAD3 3
#define DATASIZE 20

void err_quit(const char* msg);

void err_display(const char* msg);

struct BaseTask {
	std::shared_ptr<Socket> clientSocket; // 누가 보냈는가? 
	int taskType{}; // 무슨 요청인가? 
	char data[DATASIZE]{}; // 실제 데이터 
};

class GameServer {
public:
	GameServer(int port);
	//~GameServer();

	void ready();
	void run();

private:

	static DWORD WINAPI GameLogic(LPVOID arg);
	static DWORD WINAPI ClientThread(LPVOID arg);

	void HandleCardRequest(BaseTask* arg);
	void HandleCollisionRequest(BaseTask* arg);

	int retval;
	int Port;

	Socket listen_sock;

	ThreadSafeQueue<BaseTask*> threadQueue;

	CRITICAL_SECTION m_clientListCS;
	std::shared_ptr<Socket> m_clientList[2]; 

	HANDLE hThread[THREADNUM];

	struct ThreadArgs {
		GameServer* server;
		std::shared_ptr<Socket> sock;
	};

};


#endif 
