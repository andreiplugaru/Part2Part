
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
#include <iostream>

typedef void * (*THREADFUNCPTR)(void *);

#define CONNECT_CODE 2
#define DISCONNECT_CODE 3

#include "SuperNode.h"
#include "Node.h"
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
    if (listen(sd, 10) == -1) {
        perror("[server]Eroare la listen().\n");
        return Failure;
    }
    //  printf("[server]Asteptam la portul %d...\n", PORT);
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
    return NULL;
}
int main (int argc, char *argv[]) {
    //
    int nr = 0;
    char buf[255];
    if (argc < 1) {
        printf("Sintaxa: %s\n", argv[0]);
        return -1;
    }
    pthread_t new_thread;
    pthread_create(&new_thread, NULL, &showInterface, NULL);
    th.push_back(new_thread);
    currentNode->initDB();
    if (argc == 1)
    {
        currentNode = new SuperNode();
        port = PORT;
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
        if(currentNode->requestInfoFromSuperNode(inet_addr(argv[1]),htons(PORT)) == Success) {
            if (currentNode->hasAvailableSuperNodes()) {
                connectionResult = currentNode->connectToSuperNode();
                if (currentNode->shouldBeRedundantSuperNode) {
                    currentNode = SuperNode::makeRedundantSuperNode(currentNode, Network::getIp());
                }
            } else {
                NextSuperNodeResponse response = currentNode->makeNewSuperNode();
                if (response.result == Success) {
                    currentNode = new SuperNode();
                    ((SuperNode *) currentNode)->nextIpSuperNode = response.Nextip;
                    ((SuperNode *) currentNode)->isRedundantSuperNode = false;

                    currentNode->ipSuperNode = Network::getIp();
                    ((SuperNode *) currentNode)->ipOfNextRedundantSuperNode = response.NextRedundantIp;
                    currentNode->ip = Network::getIp();
                    connectionResult = Success;
                    pthread_t new_thread;
                    pthread_create(&new_thread, NULL, &pingFunction, currentNode);
                    th.push_back(new_thread);
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
            nextSuperNode.foundRatio = ((SuperNode*)currentNode)->filesFound;
            nextSuperNode.available = ((SuperNode*)currentNode)->connectedNodes.size() < MAX_CLIENTS_PER_SUPERNODE;//should be modiifed
            Network::send(tdL.cl, nextSuperNode);
        }
        else
        {
            NextSuperNodeResponse nextSuperNode;
            nextSuperNode.Nextip = currentNode->ipSuperNode;
            nextSuperNode.Nextport = currentNode->portSuperNode;
            nextSuperNode.available = true;
            Network::send(tdL.cl, nextSuperNode);
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
        //  printf("\nnew next in network order is: %d\n", newNextIp);
        NextSuperNodeResponse response;
        response.Nextip = ((SuperNode*)currentNode)->nextIpSuperNode;
        response.NextRedundantIp = ((SuperNode*)currentNode)->ipOfRedundantSuperNode;
        Network::send(tdL.cl, response);
        if(checkIsSuperNode())//current node is a super node
        {
            ((SuperNode *) currentNode)->nextIpSuperNode = newNextIp;
            ((SuperNode *) currentNode)->updateNextIpToRedundant();
        }
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &newNextIp, ipStr, INET_ADDRSTRLEN);

    }
    else if(request == ChooseAsRedunantSuperNode)
    {
        currentNode = SuperNode::makeRedundantSuperNode(currentNode,Network::getIp());
    }
    else if(request == Disconnect) {
        printf("Disconnecting...\n");
        ((SuperNode*)currentNode)->disconnect(tdL.sockaddr->sin_addr.s_addr);
    }
    else if(request == GetConnectedNodes) {
        ((SuperNode*)currentNode)->sendConnectedNodes(tdL.cl);
    }
    else if(request == SendNewNodeToRedundantSuperNode) {
        Node node;
        Network::receive(tdL.cl, node);
        node.scannedSuperNodes = new std::unordered_map<in_addr_t, int>;
        std::lock_guard<std::mutex> lock( ((SuperNode*)currentNode)->connectedNodes_mutex);
        ((SuperNode*)currentNode)->connectedNodes.push_back(&node);
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, & ((SuperNode*)currentNode)->connectedNodes[ ((SuperNode*)currentNode)->connectedNodes.size() - 1]->ip, ipStr, INET_ADDRSTRLEN);
    }
    else if(request == RemoveNodeFromRedundantSuperNode) {
        int nodeIndex;
        if (read(tdL.cl, &nodeIndex, sizeof(int)) <= 0) {
            perror("Eroare la write().\n");
        }
        if(nodeIndex > 0) {
            std::lock_guard<std::mutex> lock( ((SuperNode*)currentNode)->connectedNodes_mutex);
            ((SuperNode *) currentNode)->connectedNodes.erase(
                    ((SuperNode *) currentNode)->connectedNodes.begin() + nodeIndex);

        } }
    else if(request == Ping)
    {
        // printf("ping request\n");
    }
    else if(request == BecomeSuperNode)
    {
        printf("[debug] this node is a super node\n");
        ((SuperNode*)currentNode)->transformToNonRedundantSuperNode(tdL.cl);
    }
    else if(request == CheckFileExists)
    {
        FileRequest fileRequest;
        Network::receive(tdL.cl, fileRequest);
        Result result1 = currentNode->checkFileExists(fileRequest);
        if (write(tdL.cl, &result1, sizeof(Result)) <= 0) {
            perror("Eroare la write().\n");
        }
        if(result1 == Success) {
            fileRequest.ipOfTheNodeWithFile = Network::getIp();

        }
        Network::send(tdL.cl, fileRequest);
    }
    else if(request == RequestFileFromConnectedNode) {
        ((SuperNode*)currentNode)->receiveRequestFromConnectedNode(tdL.cl);
    }
    else if(request == RequestFileFromSuperNode)
    {
        ((SuperNode*)currentNode)->receiveRequestFromSuperNode(tdL.cl);
    }
    else if(request == SendFileInfoToRequestingSuperNode)
    {
        FileRequest fileRequest;
        Network::receive(tdL.cl, fileRequest);
        ((SuperNode*)currentNode)->notifyNodeFileFound(fileRequest);
    }
    else if(request == SuperNodeFileNotFound)
    {
        FileRequest fileRequest;
        Network::receive(tdL.cl, fileRequest);
        ((SuperNode*)currentNode)->hasRequest(fileRequest);
        ((SuperNode*)currentNode)->notifyNodeFileNotFound(fileRequest);
    }
    else if(request == NodeFileNotFound)
    {
        FileRequest fileRequest;
        Network::receive(tdL.cl, fileRequest);

        printf("Unfortunately file: %s has not been found!\n", fileRequest.fileName);
    }
    else if(request == NodeFileFound)
    {
        FileRequest fileRequest;
        Network::receive(tdL.cl, fileRequest);
        currentNode->initiateFileTransferRequest(fileRequest);
    }
    else if(request == InitiateFileTransfer)
    {
        FileRequest fileRequest;
        Network::receive(tdL.cl, fileRequest);

        currentNode->initiateFileTransferSend(tdL.cl, fileRequest);
    }
    else if(request == GetNextSuperNode)
    {
        if (write(tdL.cl, &((SuperNode*)currentNode)->nextIpSuperNode, sizeof(in_addr_t)) <= 0) {
            perror("Eroare la write().\n");
        }
    }
    else if(request == GetNextRedundantSuperNode)
    {
        if (write(tdL.cl, &((SuperNode*)currentNode)->ipOfNextRedundantSuperNode, sizeof(in_addr_t)) <= 0) {
            perror("Eroare la write().\n");
        }
    }
    else if(request == UpdateNextRedundant)
    {
        in_addr_t ipOfNode, ipofRedundantNode;
        if (read (tdL.cl,&ipOfNode,sizeof(in_addr_t)) <= 0)
        {
            perror ("Read error().\n");
        }
        if (read (tdL.cl,&ipofRedundantNode,sizeof(in_addr_t)) <= 0)
        {
            perror ("Read error().\n");
        }
        if(ipOfNode == Network::getIp())
        {
            ((SuperNode*)currentNode)->ipOfNextRedundantSuperNode = ipofRedundantNode;
            ((SuperNode*)currentNode)->updateNextRedundantIpToRedundant();
        }
        else
        {
            ((SuperNode*)currentNode)->updateNextRedundantIp(((SuperNode*)currentNode)->nextIpSuperNode,ipOfNode, ipofRedundantNode);
        }
    }
    else if(request == UpdateNextIpToRedundant)
    {
        if (read (tdL.cl,& ((SuperNode*)currentNode)->nextIpSuperNode,sizeof(in_addr_t)) <= 0)
        {
            perror ("Read error().\n");
        }
    }
    else if(request == UpdateNextRedundantIpToRedundant)
    {
        if (read (tdL.cl,& ((SuperNode*)currentNode)->ipOfNextRedundantSuperNode,sizeof(in_addr_t)) <= 0)
        {
            perror ("Read error().\n");
        }
    }
    close((intptr_t)arg);
    pthread_detach(pthread_self());
    return(NULL);
};
static void *showInterface(void * arg)
{
    int c;

    std::cout<<"Available commands"<<std::endl;
    std::cout<<"add <filename>"<<std::endl;
    std::cout<<"search <filename> [condition for size: size >/</= n]"<<std::endl;
    std::cout<<"shared"<<std::endl;
    std::cout<<"show_debug_info"<<std::endl;
    std::cout<<"exit"<<std::endl;

    std::string command;
    while(true)
    {
        std::cout<<"Write command:"<<std::endl;
        std::getline(std::cin,command);

        if(strstr(command.c_str(), "search")!=NULL)
        {
            Operators op = Nothing;
            std::string sizeCommand = command.substr(command.find(" ") + 1);
                char* commandChr = strdup(sizeCommand.c_str());
                char* token = strtok(commandChr, " ");
                std::string fileName;
                int i = 0;
                int n = 0;
                while( token != NULL ) {
                if(i == 0)
                {
                    fileName = token;
                }
              else  if(i == 1 && !strcmp(token, "size"))
                {
                break;
                }
                else if(i == 2 && (strcmp(token, ">") || strcmp(token, "<") || strcmp(token, "<=") || strcmp(token, ">=") || strcmp(token, "==")))
                {
                  if(strcmp(token, ">") == 0)
                    op = Greater;
                  else if(strcmp(token, ">=") == 0)
                      op = GreaterEqual;
                  else if(strcmp(token, "==") == 0)
                      op = Equal;
                  else if(strcmp(token, "<") == 0)
                      op = Less;
                  else if(strcmp(token, "<=") == 0)
                      op = LessEqual;
                }
                else if(i == 3)
                {
                    n = std::stoi(token);
                }
                i++;
                token = strtok(NULL, " ");
            }
            currentNode->searchFile(fileName, op, n);

        }
        else if (strstr(command.c_str(), "add")!=NULL)
        {
            std::string fileName = command.substr(command.find(' ') + 1);
            currentNode->addFiles(fileName);
        }
        else if (strstr(command.c_str(), "exit")!=NULL)
        {
            exit(0);
        }
        else if (strstr(command.c_str(), "next")!=NULL)
        {
            char str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &((SuperNode*)currentNode)->nextIpSuperNode, str, INET_ADDRSTRLEN);
            printf("ip of the next supernode: %s\n", str);

            inet_ntop(AF_INET, &((SuperNode*)currentNode)->ipOfNextRedundantSuperNode, str, INET_ADDRSTRLEN);
            printf("ip of the next redundat supernode: %s\n", str);
        }
        else if (strstr(command.c_str(), "shared")!=NULL)
        {
            currentNode->showSharedFiles();
        }
        else if (strstr(command.c_str(), "show_debug_info")!=NULL) {
            printf("[debug]Is super node: %d\n", checkIsSuperNode());
            if(dynamic_cast<SuperNode *>(currentNode) != nullptr)
            printf("[debug]Is redundant node: %d\n", ((SuperNode*)currentNode ) ->isRedundantSuperNode);
            else
                printf("[debug]Is redundant node: %d\n",0);


        }
    }
    return(NULL);
}

