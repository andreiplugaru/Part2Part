//
// Created by andrew01 on 12/14/22.
//

#ifndef HOST_DEFINES_H
#define HOST_DEFINES_H

#define PORT 2908
#define MAX_CLIENTS_PER_SUPERNODE 2
typedef struct thData{
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
    int cl; //descriptorul intors de accept
    struct sockaddr_in * sockaddr;

}thData;
//enum Codes{ Connect, Disconnect, RequestFile};
enum Result{ Success, Failure, NotSuperNode, Reject};
enum RequestType{ GetPrevSuperNode,GetNextSuperNode, ConnectToSuperNode, UpdateNextNodeNeighbour, UpdatePrevNodeNeighbour, Disconnect,
        GetNeighbourInfo, Ping, GetConnectedNodes, SendNewNodeToRedundantSuperNode, RemoveNodeFromRedundantSuperNode, ChooseAsRedunantSuperNode};
/*typedef struct Request{
    Codes code;
    Result result;
    char data[255];
};*/
#endif //HOST_DEFINES_H
