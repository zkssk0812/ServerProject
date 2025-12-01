#pragma once

#include <string>
#include <winsock2.h> 
#include <ws2tcpip.h> 
#include "ThreadSafeQueue.h" // 서버와 동일한 헤더 사용 (없으면 복사해오기)

#pragma comment(lib, "ws2_32.lib")

#define CM_REQUEST_CARD 100        // 카드를 사용했음을 알림
#define CM_REQUEST_COLLISION 101   // 말이 충돌했음을 알림
#define BUFSIZE 20
#define DATASIZE 20

// 서버와 통신할 패킷 구조체 (서버와 동일)
struct BaseTask {
    int taskType{};       // 프로토콜 (100 or 101)
    char data[DATASIZE]{}; // 데이터
};

class GameClient {
public:
    GameClient(std::string serverIp, int port);
    ~GameClient();

    bool ConnectToServer();

    // 데이터 전송 함수
    void CardSendDate(int dis, int drection, int playerNum);
    void PlayerSendDate(int id, int playerNum);

private:
    std::string ServerIp;
    int Port;
    SOCKET sock;

    int id;

    // [김윤기 담당] 스레드 및 동기화 관련 변수
    static DWORD WINAPI ClientRecvThread(LPVOID arg);      // 서버에서 받아서 큐에 넣는 스레드
    static DWORD WINAPI GameStateUpdateThread(LPVOID arg); // 큐에서 꺼내서 게임에 적용하는 스레드

    void ApplyServerSync(BaseTask* task); // 실제 동기화 로직

    ThreadSafeQueue<BaseTask*> threadQueue; // 스레드 안전 큐
    HANDLE hThread[2]; // 0: Recv, 1: Update
};