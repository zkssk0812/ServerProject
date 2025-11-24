

#include "GameClient.h" 
#include <iostream>



GameClient::GameClient(std::string serverIp, int port)
    : ServerIp(serverIp), Port(port), sock(INVALID_SOCKET) {

    WSADATA wsaData;
    int retval = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (retval != 0) {
        std::cerr << "[오류] WSAStartup() 실패: " << retval << std::endl;
    }
}

GameClient::~GameClient() {
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    WSACleanup();
}

bool GameClient::ConnectToServer() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "[오류] socket() 실패: " << WSAGetLastError() << std::endl;
        return false;
    }

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(Port);

    if (inet_pton(AF_INET, ServerIp.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "[오류] inet_pton() 실패: 유효하지 않은 IP 주소입니다." << std::endl;
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    int retval = connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (retval == SOCKET_ERROR) {
        std::cerr << "[오류] connect() 실패: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    std::cout << "[성공] 서버에 접속했습니다. (IP: " << ServerIp << ", Port: " << Port << ")" << std::endl;

    // GameStateUpdateThread (서버 수신 스레드)를 생성부분 추가

    return true;
}


void GameClient::CardSendDate(int dis, int drection) {
    if (sock == INVALID_SOCKET) {
        std::cerr << "[오류] CardSendDate: 서버에 연결되지 않았습니다." << std::endl;
        return;
    }

    char buf[BUFSIZE];
    int protocol = CM_REQUEST_CARD;

    int offset = 0;

    memcpy(buf + offset, &protocol, sizeof(int));
    offset += sizeof(int);

    memcpy(buf + offset, &dis, sizeof(int));
    offset += sizeof(int);

    memcpy(buf + offset, &drection, sizeof(int));
    offset += sizeof(int);

    int totalSize = offset;

    int retval = send(sock, buf, totalSize, 0);
    if (retval == SOCKET_ERROR) {
        std::cerr << "[오류] CardSendDate() - send() 실패: " << WSAGetLastError() << std::endl;
    }
    else {
        std::cout << "[전송] 카드 사용 (거리: " << dis << ", 방향: " << drection << ")" << std::endl;
    }
}

void GameClient::PlayerSendDate(int id, int playerNum) {
    if (sock == INVALID_SOCKET) {
        std::cerr << "[오류] PlayerSendDate: 서버에 연결되지 않았습니다." << std::endl;
        return;
    }

    char buf[BUFSIZE];
    int protocol = CM_REQUEST_COLLISION;

    int offset = 0;

    memcpy(buf + offset, &protocol, sizeof(int));
    offset += sizeof(int);

    memcpy(buf + offset, &id, sizeof(int));
    offset += sizeof(int);

    memcpy(buf + offset, &playerNum, sizeof(int));
    offset += sizeof(int);

    int totalSize = offset;

    int retval = send(sock, buf, totalSize, 0);
    if (retval == SOCKET_ERROR) {
        std::cerr << "[오류] PlayerSendDate() - send() 실패: " << WSAGetLastError() << std::endl;
    }
    else {
        std::cout << "[전송] 충돌 발생 (ID: " << id << ", 상대: " << playerNum << ")" << std::endl;
    }
}