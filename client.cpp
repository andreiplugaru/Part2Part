
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

#define CONNECT_CODE 2
#define DISCONNECT_CODE 3

#include "Network.h"
#include "Node.cpp"
#include "SuperNode.h"

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
Node hostNode;
static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
static void *showInterface(void * arg);
pthread_t th[100];
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
        printf("Accepting...\n");
        td = (struct thData *) malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;
        td->sockaddr = &from;
        pthread_create(&th[i], NULL, &treat, td);
    }
}
int port;
std::vector<Node*> connectedNodes;
Node* currentNode;
int main (int argc, char *argv[]) {
    // mesajul trimis
    int nr = 0;
    char buf[255];
    if (argc < 2) {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }
    pthread_create(&th[i], NULL, &showInterface, NULL);
    i++;

    if (argc == 2) {
        currentNode = new SuperNode();
        port = atoi(argv[1]);
        printf("Node connected to itself\n");
        currentNode->isFirstNode = true;
        currentNode->ip = htonl (INADDR_ANY);
        currentNode->port = htons(port);
        acceptConnections();
    } else {
        currentNode = new Node();
        Result connectionResult = currentNode->connectToSuperNode(inet_addr(argv[1]),htons(atoi(argv[2])));
        if(connectionResult == Success)
        {
            printf("Node connected to host\n");
        }
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
    if (read (tdL.cl, &request,sizeof(Request)) <= 0)
    {
        perror ("Eroare la read() de la client.\n");
    }
    if(request == ConnectToSuperNode) {
        if(currentNode->ipSuperNode == true)//current node is a super node
        {
            if(((SuperNode*)currentNode)->connectedNodes.size()< MAX_CLIENTS_PER_SUPERNODE)
            {
                ((SuperNode*)currentNode)->acceptNewNode(tdL.sockaddr->sin_addr.s_addr, tdL.sockaddr->sin_port, tdL.cl);
            }
            else
            {
                ((SuperNode*)currentNode)->rejectNewNode(tdL.sockaddr->sin_addr.s_addr, tdL.sockaddr->sin_port, tdL.cl);
            }
        }
        else//current node is NOT a super node
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
        }

    }
    else if(request == Disconnect)
    {
        printf("Disconnecting...\n");
        for(int i = 0; i < connectedNodes.size(); i++)
        {
            if(connectedNodes[i]->ip == tdL.sockaddr->sin_addr.s_addr)
            {
                connectedNodes.erase(connectedNodes.begin() + i);
                Request response;

                if (write(tdL.cl, &response, sizeof(Request)) <= 0) {
                    perror("Eroare la write().\n");
                }
                break;
            }
        }
    }
    pthread_detach(pthread_self());
    //raspunde((struct thData*)arg);
    /* am terminat cu acest client, inchidem conexiunea */
   // printf("Connected node\n");
    close ((intptr_t)arg);
    return(NULL);

};
static void *showInterface(void * arg)
{
    int c;
    printf("Available commands\n");
    printf("1 - Show connected peers to this node\n");
    printf("2 - Disconnect from host\n");
    printf("3 - Is super node\n");
    printf("-1 Exit\n");
    scanf("%d", &c);
    while(c != -1)
    {
        if(c == 1)
        {
            for(int i = 0; i < connectedNodes.size(); i++) {
                char str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &connectedNodes[i]->ip, str, INET_ADDRSTRLEN);
                printf("%d: %s %s ", i, str, connectedNodes[i]->name.c_str());
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
        scanf("%d", &c);
    }
    return(NULL);
}