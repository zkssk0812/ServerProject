#include "GameClient.h"
#include "function.h" // Player1, Player2 전역변수 접근용
#include <iostream>

GameClient::GameClient(std::string serverIp, int port)
    : ServerIp(serverIp), Port(port), sock(INVALID_SOCKET) {
}

GameClient::~GameClient() {
    if (sock != INVALID_SOCKET) closesocket(sock);
    WSACleanup();
}

bool GameClient::ConnectToServer() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return false;

    sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, ServerIp.c_str(), &serveraddr.sin_addr);
    serveraddr.sin_port = htons(Port);

    if (connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR) {
        printf("[클라이언트] 서버 연결 실패\n");
        return false;
    }

    char buf[4];

    recv(sock, buf, 4, 0);

    memcpy(&id, buf, sizeof(int));

    printf("[클라이언트] 서버 연결 성공!\n");

    // [김윤기 핵심] 수신 스레드와 업데이트 스레드 실행
    hThread[0] = CreateThread(NULL, 0, ClientRecvThread, this, 0, NULL);
    hThread[1] = CreateThread(NULL, 0, GameStateUpdateThread, this, 0, NULL);

    return true;
}

// [스레드 1] 서버로부터 데이터를 받아 큐에 넣는 역할 (Producer)
DWORD WINAPI GameClient::ClientRecvThread(LPVOID arg) {
    GameClient* client = (GameClient*)arg;
    char buf[BUFSIZE];

    while (true) {
        int retval = recv(client->sock, buf, BUFSIZE, 0);
        if (retval == SOCKET_ERROR || retval == 0) break;

        // 패킷 파싱: [Protocol(4byte)] + [Data...]
        if (retval >= sizeof(int)) {
            int protocol;
            memcpy(&protocol, buf, sizeof(int));

            BaseTask* task = new BaseTask();
            task->taskType = protocol;

            // 데이터 부분 복사 (프로토콜 이후 데이터)


            int dataLen = retval - sizeof(int); // 전체 길이에서 프로토콜 크기(4) 뺌
            if (dataLen > 0) {
                if (dataLen > DATASIZE) dataLen = DATASIZE; // 안전장치
                memcpy(task->data, buf + sizeof(int), dataLen);
            }


            // 그냥 바로 전달
            client->ApplyServerSync(task);
        }
    }
    return 0;
}

// [스레드 2] 큐에서 데이터를 꺼내 게임에 적용하는 역할 (Consumer) - 김윤기 담당 -> 필요 없음


// [핵심 로직] 서버 데이터를 실제 게임 오브젝트(Player1, Player2)에 반영
void GameClient::ApplyServerSync(BaseTask* task) {
    // task->data 구조: [id] + [말 번호] + [RealData...]
    char* realData = task->data + sizeof(int);
    int data[4]{};

    for (int i = 0; i < 4; ++i) {
        int offset = i * sizeof(int);
        memcpy(data + i, realData + offset, sizeof(int)); // 데이터를 미리 처리해서 저장
    }

    int Player = data[0];
    int PlayerNum = data[1];

    // 실제 데이터는 playerNum(4byte) 뒤에 있음


    if (task->taskType == CM_REQUEST_CARD) {
        // 카드 이동 정보: [Distance(int)] + [Direction(int)]
        int dist = data[2];
        int direction = data[3];

        printf("[Sync] Player %d Move: Dist %d, Dir %d\n", Player, dist, direction);

        // [핵심] 전역 변수(Player1, Player2)에 접근하여 움직임 동기화
        // function.h에 선언된 Player1, Player2 객체를 사용합니다.
        if (Player == 0) {
            // Player 0번이 움직였다면 -> 내 화면의 Player1[0] 객체를 움직임
            Player1[0].changeDirection(direction, dist);
        }
        else if (Player == 1) {
            // Player 1번이 움직였다면 -> 내 화면의 Player2[0] 객체를 움직임
            Player2[0].changeDirection(direction, dist);
        }
    }
    else if (task->taskType == CM_REQUEST_COLLISION) {
        // 충돌 정보 처리
        // 예: 누가 충돌했는지 식별 후 die() 호출 등
        int DeletePlayer = data[2];
        int DeletePlayerNum = data[3];

        if (DeletePlayer == 0) {
            Player1[DeletePlayerNum].die(); // 예시 로직
        }
        else if (DeletePlayer == 1) {
            Player2[DeletePlayerNum].die();
        }
        printf("[Sync] Player %d Collision!\n", Player);
    }
}

// ---------------------------------------------------------
// [전송 함수] 여기서부터는 Client가 Server로 보낼 때 사용
// ---------------------------------------------------------

void GameClient::CardSendDate(int dis, int drection, int playerNum) {
    if (sock == INVALID_SOCKET) return;

    // 프로토콜(100) + 거리(int) + 방향(int)
    int protocol = CM_REQUEST_CARD;
    char buf[BUFSIZE];
    int offset = 0;

    // 1. 프로토콜 타입 복사
    memcpy(buf + offset, &protocol, sizeof(int));
    offset += sizeof(int);

    // 2. id
    memcpy(buf + offset, &id, sizeof(int));
    offset += sizeof(int);

    // 3. 말
    memcpy(buf + offset, &playerNum, sizeof(int));
    offset += sizeof(int);

    // 4. 데이터 복사 (거리)
    memcpy(buf + offset, &dis, sizeof(int));
    offset += sizeof(int);

    // 5. 데이터 복사 (방향)
    memcpy(buf + offset, &drection, sizeof(int));
    offset += sizeof(int);

    // 서버로 전송
    send(sock, buf, offset, 0);
}

void GameClient::PlayerSendDate(int id, int playerNum) {
    if (sock == INVALID_SOCKET) return;

    // 프로토콜(101) + ID(int) + PlayerNum(int) -> 상황에 맞춰 데이터 구성
    int protocol = CM_REQUEST_COLLISION;
    char buf[BUFSIZE];
    int offset = 0;

    memcpy(buf + offset, &protocol, sizeof(int));
    offset += sizeof(int);

    memcpy(buf + offset, &id, sizeof(int));
    offset += sizeof(int);

    memcpy(buf + offset, &playerNum, sizeof(int));
    offset += sizeof(int);

    send(sock, buf, offset, 0);
}


// [김윤기 담당] 스레드 2 - 현재 구조상 RecvThread에서 바로 처리하므로 비워둡니다.
// 링커 오류 방지를 위해 껍데기만 유지합니다.
DWORD WINAPI GameClient::GameStateUpdateThread(LPVOID arg) {
    // 할 일 없음 (Recv 스레드에서 ApplyServerSync를 직접 호출하고 있으므로)
    return 0;
}