
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <vector>
#include <cstring>
#include <ifaddrs.h>
typedef void * (*THREADFUNCPTR)(void *);

#define CONNECT_CODE 2
#define DISCONNECT_CODE 3

#include "Network.cpp"
#include "Node.cpp"
#include "SuperNode.h"

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
Node hostNode;
static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
static void *showInterface(void * arg);
std::vector<pthread_t> th;
int i = 0;
Result acceptConnections() {
    int sd;
    struct sockaddr_in from;
    struct sockaddr_in server;
    int on = 1;
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la socket().\n");
        return Failure;
    }
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("[server] Eroare la bind2().\n");
        return Failure;
    }
    if (listen(sd, 2) == -1) {
        perror("[server]Eroare la listen().\n");
        return Failure;
    }
    printf("[server]Asteptam la portul %d...\n", PORT);
    while (1) {
        int client;
        thData *td; //parametru functia executata de thread
        int length = sizeof(from);
        fflush(stdout);
        if ((client = accept(sd, (struct sockaddr *) &from, reinterpret_cast<socklen_t *>(&length))) < 0) {
            perror("[server]Eroare la accept().\n");
            continue;
        }
        td = (struct thData *) malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;
        td->sockaddr = &from;
        pthread_t new_thread;
        pthread_create(&new_thread, NULL, &treat, td);
        th.push_back(new_thread);
    }
}
int port;
std::vector<Node*> connectedNodes;
Node* currentNode;
bool checkIsSuperNode()
{
    return dynamic_cast<SuperNode *>(currentNode) != nullptr && !((SuperNode*)(currentNode))->isRedundantSuperNode;
}
static void* pingFunction(void *)
{
    ((SuperNode*)currentNode)->ping();
}
int main (int argc, char *argv[]) {
    //
    int nr = 0;
    char buf[255];
    if (argc < 2) {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }
    pthread_t new_thread;
    pthread_create(&new_thread, NULL, &showInterface, NULL);
    th.push_back(new_thread);
    if (argc == 2)
    {
        currentNode = new SuperNode();
        port = atoi(argv[1]);
        printf("Node connected to itself\n");
        currentNode->isFirstNode = true;
        currentNode->ip = Network::getIp();
        currentNode->ipSuperNode = Network::getIp();
        currentNode->port = htons(port);
        ((SuperNode*)currentNode)->nextIpSuperNode = Network::getIp();
        ((SuperNode*)currentNode)->isRedundantSuperNode = false;
        pthread_t new_thread;
        pthread_create(&new_thread, NULL, &pingFunction, currentNode);
        th.push_back(new_thread);
        acceptConnections();
    } else
    {
        Result connectionResult;
        currentNode = new Node();
        if(currentNode->requestInfoFromSuperNode(inet_addr(argv[1]),htons(atoi(argv[2]))) == Success) {
            if (currentNode->hasAvailableSuperNodes()) {
                connectionResult = currentNode->connectToSuperNode();
                if (currentNode->shouldBeRedundantSuperNode) {
                    currentNode = SuperNode::makeRedundantSuperNode(currentNode);
                }
            } else {
                NextSuperNodeResponse response = currentNode->makeNewSuperNode();
                if (response.result == Success) {
                    currentNode = new SuperNode();
                    ((SuperNode *) currentNode)->nextIpSuperNode = response.Nextip;
                    ((SuperNode *) currentNode)->isRedundantSuperNode = false;
                    currentNode->ipSuperNode = Network::getIp();
                    ((SuperNode *) currentNode)->ipOfNextRedundantSuperNode = response.NextRedundantIp;
                    printf("received redundatn ip = %d", response.NextRedundantIp);
                    currentNode->ip = Network::getIp();
                    connectionResult = Success;
                    pthread_t new_thread;
                    pthread_create(&new_thread, NULL, &pingFunction, currentNode);
                    th.push_back(new_thread);
                    printf("This node is a super node!\n");
                } else {
                    connectionResult = Failure;
                    perror("Error when trying to make this node a super node!\n");
                }
            }
        }
        else
        {
            perror("Connection failed!");
        }
        if(connectionResult == Success)
        {
            acceptConnections();
            printf("Node connected to super node\n");
        }
        currentNode->scannedSuperNodes->clear();
    }


}
static void *treat(void * arg)
{
    char utilizatorCurent[255];
    int mesaj;
    struct thData tdL;
    tdL= *((struct thData*)arg);
    fflush (stdout);
    RequestType request;
    if (read (tdL.cl, &request,sizeof(RequestType)) <= 0)
    {
        perror ("Eroare la read() de la client.\n");
    }
    //printf("TEST\n");
    Result result;
    result = Success;

    if (write(tdL.cl, &result, sizeof(Result)) <= 0) {
        perror("Eroare la write().\n");
    }
    if(request == GetNeighbourInfo)
    {
        if(checkIsSuperNode())//current node is a super node
        {
            NextSuperNodeResponse nextSuperNode;
            nextSuperNode.Nextip = ((SuperNode*)currentNode)->nextIpSuperNode;
            nextSuperNode.Nextport = ((SuperNode*)currentNode)->nextPortSuperNode;
            nextSuperNode.isAlone = ((SuperNode*)currentNode)->isAlone;
            nextSuperNode.NextRedundantIp = ((SuperNode*)currentNode)->ipOfRedundantSuperNode;
            printf("nextSuperNode.NextRedundantIp = %d",nextSuperNode.NextRedundantIp);
            nextSuperNode.available = ((SuperNode*)currentNode)->connectedNodes.size() < MAX_CLIENTS_PER_SUPERNODE;//should be modiifed
            if (write(tdL.cl, &nextSuperNode, sizeof(NextSuperNodeResponse)) <= 0) {
                perror("Eroare la write().\n");
            }
        }
        else
        {
            NextSuperNodeResponse nextSuperNode;
            nextSuperNode.Nextip = currentNode->ipSuperNode;
            nextSuperNode.Nextport = currentNode->portSuperNode;
            nextSuperNode.available = true;
            if (write(tdL.cl, &nextSuperNode, sizeof(NextSuperNodeResponse)) <= 0) {
                perror("Eroare la write().\n");
            }
        }
    }
    else if(request == ConnectToSuperNode) {
        if(checkIsSuperNode())//current node is a super node
        {
            ((SuperNode*)currentNode)->acceptNewNode(tdL.sockaddr->sin_addr.s_addr, tdL.sockaddr->sin_port, tdL.cl);
        }
        else
            printf("A node tried to connect, but this isn't a supernode\n");
        }
    else if(request == UpdateNextNodeNeighbour)
    {
        in_addr_t newNextIp;
        if (read (tdL.cl, &newNextIp,sizeof(in_addr_t)) <= 0)
        {
            perror ("Eroare la read() de la client.\n");
        }
        printf("\nnew next in network order is: %d\n", newNextIp);
        NextSuperNodeResponse response;
        response.Nextip = currentNode->ip;
        response.NextRedundantIp = ((SuperNode*)currentNode)->ipOfRedundantSuperNode;
        if (write (tdL.cl, &response,sizeof(NextSuperNodeResponse)) <= 0)
        {
            perror ("Eroare la write() de la client.\n");
        }
        if(checkIsSuperNode())//current node is a super node
            ((SuperNode*)currentNode)->nextIpSuperNode = newNextIp;
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &newNextIp, ipStr, INET_ADDRSTRLEN);
        printf("\nnew next ip is: %s\n", ipStr);
    }
    else if(request == ChooseAsRedunantSuperNode)
    {
        printf("\nThis node has been chosen as supernode!\n");
        currentNode = SuperNode::makeRedundantSuperNode(currentNode);
    }
        /*else//current node is NOT a super node
        {
            int result = NotSuperNode;
            if (write(tdL.cl, &result, sizeof(int)) <= 0) {
                perror("Eroare la write().\n");
            }
            Node nextSuperNode;
            nextSuperNode.ip = currentNode->ipSuperNode;
            nextSuperNode.port = currentNode->port;
            if (write(tdL.cl, &nextSuperNode, sizeof(Node)) <= 0) {
                perror("Eroare la write().\n");
            }
        }*/
    else if(request == Disconnect) {
        printf("Disconnecting...\n");
        ((SuperNode*)currentNode)->disconnect(tdL.sockaddr->sin_addr.s_addr);
    }
    else if(request == GetConnectedNodes) {
        ((SuperNode*)currentNode)->sendConnectedNodes(tdL.cl);
        }
    else if(request == SendNewNodeToRedundantSuperNode) {
        Node node;
        if (read(tdL.cl, &node, sizeof(Node)) <= 0) {
            perror("Eroare la write().\n");
        }

        node.scannedSuperNodes = new std::unordered_map<in_addr_t, double>;
        ((SuperNode*)currentNode)->connectedNodes.push_back(&node);
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, & ((SuperNode*)currentNode)->connectedNodes[ ((SuperNode*)currentNode)->connectedNodes.size() - 1]->ip, ipStr, INET_ADDRSTRLEN);
        printf("ip[last2] = %s\n", ipStr);
    }
    else if(request == RemoveNodeFromRedundantSuperNode) {
       int nodeIndex;
        if (read(tdL.cl, &nodeIndex, sizeof(int)) <= 0) {
            perror("Eroare la write().\n");
        }
        if(nodeIndex > 0)
            ((SuperNode*)currentNode)->connectedNodes.erase(((SuperNode*)currentNode)->connectedNodes.begin() + nodeIndex);
    }
    else if(request == Ping)
    {
       // printf("ping request\n");
    }
    else if(request == BecomeSuperNode)
    {
        ((SuperNode*)currentNode)->transformToNonRedundantSuperNode(tdL.cl);
    }
    else if(request == RequestFileFromConnectedNode) {
     //   printf("    else if(request == RequestFileFromConnectedNode)\n");
        FileRequest fileRequest;
        if (read(tdL.cl, &fileRequest, sizeof(FileRequest)) <= 0) {
            perror("Eroare la write().\n");
        }
        printf("requested file is %s\n", fileRequest.fileName);
        fileRequest.ipOfTheSuperNodeRequesting = Network::getIp();
        Result result1 = currentNode->checkFileExists(fileRequest);
        if(result1 == Success) {
            fileRequest.ipOfTheNodeWithFile = Network::getIp();
            if (write(tdL.cl, &result1, sizeof(Result)) <= 0) {
                perror("Eroare la write().\n");
            }
            if (write(tdL.cl, &fileRequest, sizeof(FileRequest)) <= 0) {
                perror("Eroare la write().\n");
            }
        }
       else {
            bool found = false;
            for (int i = 0; i < ((SuperNode *) currentNode)->connectedNodes.size(); i++) {

                if (((SuperNode *) currentNode)->connectedNodes[i]->ip != fileRequest.ipOfTheNodeRequesting) {
                    int sd2;
                    Result resultRequestFile = Network::makeRequest(((SuperNode *) currentNode)->connectedNodes[i]->ip,
                                                                    htons(atoi("2908")), RequestFileFromConnectedNode,
                                                                    sd2);
                    if (resultRequestFile == Success) {
                        if (write(sd2, &fileRequest, sizeof(FileRequest)) <= 0) {
                            perror("Eroare la write().\n");
                        }
                        Result result1;
                        if (read(sd2, &result1, sizeof(Result)) <= 0) {
                            perror("Eroare la write().\n");
                        }
                        if (read(sd2, &fileRequest, sizeof(FileRequest)) <= 0) {
                            perror("Eroare la write().\n");
                        }
                        close(sd2);
                        if (fileRequest.ipOfTheNodeWithFile != 0) {
                            found = true;
                            Result result1 = Success;
                            if (write(tdL.cl, &result1, sizeof(Result)) <= 0) {
                                perror("Eroare la write().\n");
                            }
                            if (write(tdL.cl, &fileRequest, sizeof(FileRequest)) <= 0) {
                                perror("Eroare la write().\n");
                            }
                            break;
                        }
                    }
                }
            }
            if (!found) {
                printf("File was not found once\n");
                Result result1 = SearchInOtherSuperNodes;
                if (write(tdL.cl, &result1, sizeof(Result)) <= 0) {
                    perror("Eroare la write().\n");
                }
                close(tdL.cl);
                int sd2;
                Result resultRequestFile = Network::makeRequest(((SuperNode *) currentNode)->nextIpSuperNode,
                                                                htons(atoi("2908")), RequestFileFromSuperNode,
                                                                sd2);
                if (resultRequestFile == Success) {
                    if (write(tdL.cl, &fileRequest, sizeof(FileRequest)) <= 0) {
                        perror("Eroare la write().\n");
                    }
                }
                close(sd2);
            }
        }
    }
    else if(request == RequestFileFromSuperNode)
    {
        FileRequest fileRequest;
        if (read(tdL.cl, &fileRequest, sizeof(FileRequest)) <= 0) {
            perror("Eroare la write().\n");
        }

        if(!((SuperNode *) currentNode)->hasRequest(fileRequest)) {
            bool found = false;
            for (int i = 0; i < ((SuperNode *) currentNode)->connectedNodes.size(); i++) {
                if (((SuperNode *) currentNode)->connectedNodes[i]->ip != fileRequest.ipOfTheNodeRequesting) {
                    int sd2;
                    Result resultRequestFile = Network::makeRequest(((SuperNode *) currentNode)->connectedNodes[i]->ip,
                                                                    htons(atoi("2908")), RequestFileFromConnectedNode,
                                                                    sd2);
                    if (resultRequestFile == Success) {
                        if (write(sd2, &fileRequest, sizeof(FileRequest)) <= 0) {
                            perror("Eroare la write().\n");
                        }
                        Result result1;
                        if (read(sd2, &result1, sizeof(Result)) <= 0) {
                            perror("Eroare la write().\n");
                        }
                        if (read(sd2, &fileRequest, sizeof(FileRequest)) <= 0) {
                            perror("Eroare la write().\n");
                        }
                        close(sd2);
                        if(result1 == Success)
                        {
                            found = true;
                            fileRequest.ipOfTheNodeWithFile = ((SuperNode *) currentNode)->connectedNodes[i]->ip;
                            currentNode->sendFileToRequestingSuperNode(fileRequest);
                            break;
                        }
                    }
                }
            }
            if(!found) {
                ((SuperNode *) currentNode)->pendingRequests.push_back(fileRequest);
/*
                Result result1 = SearchInOtherSuperNodes;
                    if (write(tdL.cl, &result1, sizeof(Result)) <= 0) {
                        perror("Eroare la write().\n");
                    }

                    close(tdL.cl);*/

                    int sd2;
                    Result resultRequestFile = Network::makeRequest(((SuperNode *) currentNode)->nextIpSuperNode,
                                                                    htons(atoi("2908")), RequestFileFromSuperNode,
                                                                    sd2);
                    if (resultRequestFile == Success) {
                        if (write(sd2, &fileRequest, sizeof(FileRequest)) <= 0) {
                            perror("Eroare la write().\n");
                        }
                    }
                    close(sd2);
            }
        }
        else
        {
            ((SuperNode*)currentNode)->notifySuperNodeFileNotFound(fileRequest);
        }
    }
    else if(request == SendFileInfoToRequestingSuperNode)
    {
        FileRequest fileRequest;
        if (read(tdL.cl, &fileRequest, sizeof(FileRequest)) <= 0) {
            perror("Eroare la write().\n");
        }
        //printf("file with name: %s was found\n", fileRequest.fileName);
        ((SuperNode*)currentNode)->notifyNodeFileFound(fileRequest);
    }
    else if(request == SuperNodeFileNotFound)
    {
        FileRequest fileRequest;
        if (read(tdL.cl, &fileRequest, sizeof(FileRequest)) <= 0) {
            perror("Eroare la write().\n");
        }
        ((SuperNode*)currentNode)->hasRequest(fileRequest);
        ((SuperNode*)currentNode)->notifyNodeFileNotFound(fileRequest);
    }
    else if(request == NodeFileNotFound)
    {
        FileRequest fileRequest;
        if (read(tdL.cl, &fileRequest, sizeof(FileRequest)) <= 0) {
            perror("Eroare la write().\n");
        }
        printf("Unfortunately file: %s has not been found!\n", fileRequest.fileName);
    }
    else if(request == NodeFileFound)
    {
        FileRequest fileRequest;
        if (read(tdL.cl, &fileRequest, sizeof(FileRequest)) <= 0) {
            perror("Eroare la write().\n");
        }
        printf("The requested file has been found!\n");
        currentNode->initiateFileTransferRequest(fileRequest);
    }
    else if(request == InitiateFileTransfer)
    {
        FileRequest fileRequest;
        if (read(tdL.cl, &fileRequest, sizeof(FileRequest)) <= 0) {
            perror("Eroare la write().\n");
        }
        currentNode->initiateFileTransferSend(tdL.cl, fileRequest);
    }
    close((intptr_t)arg);
    pthread_detach(pthread_self());
    //raspunde((struct thData*)arg);
    /* am terminat cu acest client, inchidem conexiunea */
   // printf("Connected node\n");
    return(NULL);
};
static void *showInterface(void * arg)
{
    int c;
    printf("Available commands\n");
    printf("1 - Show connected peers to this node\n");
    printf("2 - Disconnect from host\n");
    printf("3 - Is super node\n");
    printf("4 - Show the ip of its supernode\n");
    printf("5 - Show the ip of the next supernode\n");
    printf("6 - Add a file to the network\n");
    printf("7 - Search for the file\n");
    printf("8 - Show available files\n");
    printf("-1 Exit\n");
    scanf("%d", &c);
    while(c != -1)
    {
        if(c == 1 && checkIsSuperNode())
        {
            printf("there are %d connected node\n",((SuperNode*)currentNode)->connectedNodes.size());
            for(int i = 0; i < ((SuperNode*)currentNode)->connectedNodes.size(); i++) {
                char str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &((SuperNode*)currentNode)->connectedNodes[i]->ip, str, INET_ADDRSTRLEN);
                printf("%d: %s ", i, str);
            }
        }
       else if(c == 2)
        {
            if(currentNode->disconnectFromHost(hostNode.ip, hostNode.port) == Success) {
                printf("Disconnected successfully");
                hostNode.ip = 0;
                hostNode.port = 0;
            }
        }
        else if(c == 3)
        {
            if(dynamic_cast<SuperNode*>(currentNode) == nullptr)
            {
                printf("This is not a super node\n");
            }
            else
            {
                printf("This is a super node\n");

            }
        }
        else if(c == 4) {
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &currentNode->ipSuperNode, str, INET_ADDRSTRLEN);
            printf("ip of its supernode: %s\n", str);
        }
        else if(c == 5) {
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &((SuperNode*)currentNode)->nextIpSuperNode, str, INET_ADDRSTRLEN);
            printf("ip of the next supernode: %s\n", str);

        }
        else if(c == 6) {
            currentNode->addFiles();
        }
        else if(c == 7) {
            currentNode->searchFile();
        }
        else if(c == 8) {
            currentNode->showSharedFiles();
        }

        scanf("%d", &c);
    }
    return(NULL);
}

