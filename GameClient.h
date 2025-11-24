#pragma once



#include <string>
#include <winsock2.h> 
#include <ws2tcpip.h> 


#define CM_REQUEST_CARD 100        // 카드를 사용했음을 알림
#define CM_REQUEST_COLLISION 101   // 말이 충돌했음을 알림


#define BUFSIZE 512

class GameClient {
public:
    GameClient(std::string serverIp, int port);

    ~GameClient();

   
    bool ConnectToServer();


    void CardSendDate(int dis, int drection);


    void PlayerSendDate(int id, int playerNum);


private:
    std::string ServerIp;   
    int Port;               
    SOCKET sock;            

    // GameStateUpdateThread 및 ThreadSafeQueue 추가 구현
};