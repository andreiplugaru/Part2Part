#ifndef HOST_SUPERNODE_H
#define HOST_SUPERNODE_H

#include <pthread.h>
#include "Node.h"
#include "Request.h"
#include "Network.h"
#include <vector>
#include <mutex>

typedef void * (*THREADFUNCPTR)(void *);

class SuperNode : public Node {
public:
    in_addr_t nextIpSuperNode;
    in_addr_t prevIpSuperNode;
    in_port_t nextPortSuperNode;
    in_addr_t ipOfNextRedundantSuperNode;
    std::vector<Node *> connectedNodes;
    bool isRedundantSuperNode;
    in_addr_t ipOfRedundantSuperNode;
    bool isAlone;
    std::vector<FileRequest> pendingRequests;
    std::mutex pendingRequests_mutex;
    std::mutex connectedNodes_mutex;
    int filesFound = 0;
    Result receiveRequestFromSuperNode(FileRequest fileRequest);
    Result receiveRequestFromConnectedNode(FileRequest fileRequest);
    Result checkForFileInConnectedNodes(FileRequest fileRequest);
    Result checkForFileInNextSuperNode(FileRequest fileRequest);
    bool hasRequest(FileRequest fileRequest);
    void notifySuperNodeFileNotFound(FileRequest fileRequest);

    /*static SuperNode* getNextSuperNode(in_addr_t ip, in_port_t port)
{
    int sd;
    struct sockaddr_in server;
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket error().\n");
        return NULL;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = ip;
    server.sin_port = port;
    if (connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {//
        perror("Connect error().\n");
        return NULL;
    }
    if (write (sd, &request,sizeof(Request)) <= 0)
    {
        perror ("Write error().\n");
        return NULL;
    }
    if (read (sd, &request,sizeof(Request)) <= 0)
    {
        perror ("Read error().\n");
        return NULL;
    }
    if(request.result == Success)
    {
        printf("Connected to super node\n");
        hostNode.ip = Ip;
        hostNode.port = port;
    }
    else if(request.result == NotSuperNode)
    {
        Node superNode;
        if (read (sd, &superNode,sizeof(Node)) <= 0)
        {
            perror ("Read error().\n");
            return Failure;
        }
        close(sd);
        return connectToHost(superNode.ip, superNode.port);
    }
    else if(request.result == Failure)
    {
        perror ("Unknown host error().\n");
        close(sd);
        return Failure;
    }
}*/
SuperNode(){
        ipOfRedundantSuperNode = 0;
}
    void removeConnectedNodesFromSuperNode(int id)
    {
        int sd;
        Result result = Network::makeRequest(ipOfRedundantSuperNode, htons(atoi("2908")), RequestType::RemoveNodeFromRedundantSuperNode, sd);
        if(result == Success)
        {
            int numberNodes;
            if (read (sd,&id,sizeof(int)) <= 0)
            {
                perror ("Read error().\n");
            }

        }
        close(sd);
    }

    int disconnect(in_addr_t ip)
    {

        int id = -1;
        for (int i = 0; i < connectedNodes.size(); i++) {
            if (connectedNodes[i]->ip == getIp()) {
               std::lock_guard<std::mutex> lock(connectedNodes_mutex);
                connectedNodes.erase(connectedNodes.begin() + i);
             //   const std::lock_guard<std::mutex> unlock(connectedNodes_mutex);

                if(ipOfRedundantSuperNode == getIp()) {
                    ipOfRedundantSuperNode = 0;
                }
                id = i;
                if(ipOfRedundantSuperNode != 0)
                    removeConnectedNodesFromSuperNode(id);

                break;
            }
        }
        if(ipOfRedundantSuperNode == 0)
             chooseAnotherRedundantSuperNode();
    }
    void chooseAnotherRedundantSuperNode()
    {
        ipOfRedundantSuperNode = 0;
        if(connectedNodes.size() == 0) {
            printf("There aren't any other connected nodes\n");
            return;
        }
            int sd;
        char ipStr[INET_ADDRSTRLEN];
        Result result = Failure;
        int i = 0;
        while (result == Failure && i<connectedNodes.size()) {

            inet_ntop(AF_INET, &connectedNodes[i]->ip, ipStr, INET_ADDRSTRLEN);
            printf("ip of connectedNodes[1]->ip: %s\n", ipStr);
            Result result = Network::makeRequest(connectedNodes[i]->ip, htons(atoi("2908")), ChooseAsRedunantSuperNode, sd);
            if (result == Success) {
                ipOfRedundantSuperNode = connectedNodes[i]->ip;
            }
            i++;
        }
        printf("new redundant super node in chooseAnotherRedundantSuperNode is %d\n", ipOfRedundantSuperNode);
        close(sd);
    }

    void getNextIp();

    static Node* makeRedundantSuperNode(Node* currentNode, in_addr_t ip)
    {
        Node *newNode = new SuperNode();
        ((SuperNode *) newNode)->isRedundantSuperNode = true;
        newNode->ipSuperNode = currentNode->ipSuperNode;
        newNode->ip = Network::getIp();
        ((SuperNode *) newNode)->getConnectedNodesFromSuperNode();
        ((SuperNode *) newNode)->getNextIp();
        ((SuperNode *) newNode)->getNextRedundantIp();

        return newNode;
    }
    Result acceptNewNode(in_addr_t ip, in_port_t port, int sd) {
        if (this->connectedNodes.size() < MAX_CLIENTS_PER_SUPERNODE) {
            Node *newNode = new Node();
            newNode->ip = ip;
            newNode->port = port;
            std::lock_guard<std::mutex> lock(connectedNodes_mutex);
            connectedNodes.push_back(newNode);
            //const std::lock_guard<std::mutex> unlock(connectedNodes_mutex);

            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &ip, ipStr, INET_ADDRSTRLEN);
            int result = Success;
           /* if (write(sd, &result, sizeof(Result)) <= 0) {
                perror("Eroare la write().\n");
            }*/
            AcceptSuperNodeResponse response;
            response.result = Success;
            response.shouldBeRedundantSuperNode = connectedNodes.size() == 1;
        /*    printf("connectedNodes.size() = %d\n",connectedNodes.size());
            printf("response.shouldBeRedundantSuperNode = %d\n",  response.shouldBeRedundantSuperNode);*/
            if(this->ipOfRedundantSuperNode != 0)
            {
                int sd1;
                Result result1 = Network::makeRequest(ipOfRedundantSuperNode, htons(atoi("2908")), RequestType::SendNewNodeToRedundantSuperNode, sd1);
             //   printf("after   Result result1 = makeRequest(ipOfRedundantSuperNode\n");
                if(result1 == Success)
                {
                   // connectedNodes[connectedNodes.size() - 1]->scannedSuperNodes->clear();
                  //  connectedNodes[i]->scannedSuperNodes->clear();

                    Network::send(sd1, *connectedNodes[connectedNodes.size() - 1]);

                }
                close(sd1);
            }
            if(response.shouldBeRedundantSuperNode) {
                this->ipOfRedundantSuperNode = ip;
                this->updateNextRedundantIp(nextIpSuperNode,nextIpSuperNode, ipOfRedundantSuperNode);

            }
           Network::send(sd, response);
        }
        close(sd);
        return Success;
    }
    Result rejectNewNode(in_addr_t ip, in_port_t port, int sd) {
        int result = Reject;
        if (write(sd, &result, sizeof(int)) <= 0) {
            perror("Eroare la write().\n");
        }
        if (write(sd, &result, sizeof(Result)) <= 0) {
            perror("Eroare la write().\n");
        }
        close(sd);
    }
    void sendConnectedNodes(int sd)
 {
    int numberNodes = connectedNodes.size();
     if (write(sd, &numberNodes, sizeof(int)) <= 0) {
         perror("Eroare la write().\n");
     }
     for(int i = 0; i < numberNodes; i++)
     {
         connectedNodes[i]->scannedSuperNodes->clear();
         Network::send(sd, *connectedNodes[i]);

     }
 }
    void* ping()
{
    while (true)
    {
        std::vector<in_addr_t> disconnectedIps;
        for (int i = 0; i < connectedNodes.size(); i++) {
            int sd;
            Result check = Network::makeRequest(connectedNodes[i]->ip, htons(atoi("2908")), Ping, sd);
            close(sd);
            if (check == Failure) {
                printf("A node has disconnected!\n");
                disconnectedIps.push_back(connectedNodes[i]->ip);
            }
        }
        for (int i = 0; i < disconnectedIps.size(); i++) {
            disconnect(disconnectedIps[i]);
        }
        int sd;

        if(nextIpSuperNode != getIp()) {
            Result checkSuperNode = Network::makeRequest(nextIpSuperNode, htons(atoi("2908")), Ping, sd);
            close(sd);
            if (checkSuperNode == Failure) {
                printf("Next supernode has disconnected!\n");
                Result resultRedundant = Network::makeRequest(ipOfNextRedundantSuperNode, htons(atoi("2908")),
                                                              BecomeSuperNode, sd);
                if (resultRedundant == Success) {
                    nextIpSuperNode = ipOfNextRedundantSuperNode;
                    if (read(sd, &ipOfNextRedundantSuperNode, sizeof(in_addr_t)) <= 0) {
                        perror("Eroare la write().\n");
                    }
                    printf("ipOfRedundantSuperNode = %d\n", ipOfRedundantSuperNode);
                    printf("nextIpSuperNode = %d\n", nextIpSuperNode);
                    updateNextIpToRedundant();
                    updateNextRedundantIpToRedundant();


                }
                close(sd);

            }
        }
        if(ipOfRedundantSuperNode != 0) {
            Result checkRedundantNode = Network::makeRequest(ipOfRedundantSuperNode, htons(atoi("2908")), Ping, sd);
            close(sd);
            if (checkRedundantNode == Failure) {
               // printf("checkRedundantNode == Failure\n");
                chooseAnotherRedundantSuperNode();
                close(sd);

            }
        }
        sleep(10);
    }
    return NULL;
}

/*This function is used if the current node is a redundant super node*/
    void getConnectedNodesFromSuperNode()
{
    int sd;
    Result result = Network::makeRequest(ipSuperNode, htons(atoi("2908")), RequestType::GetConnectedNodes, sd);
    if(result == Success)
    {
        int numberNodes;
        if (read (sd,&numberNodes,sizeof(int)) <= 0)
        {
            perror ("Read error().\n");
        }
        for(int i = 0; i < numberNodes; i++)
        {
            Node currentNode;
            Network::receive(sd, currentNode);
            char ipStr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &currentNode.ip, ipStr, INET_ADDRSTRLEN);
            currentNode.scannedSuperNodes = new std::unordered_map<in_addr_t, int>();
            connectedNodes.push_back(&currentNode);
        }
    }
    close(sd);
}
void transformToNonRedundantSuperNode(int sd)
{
    printf("This node transformed to nonredunat supernode!\n");

    isRedundantSuperNode = false;
    connectedNodes.erase(connectedNodes.begin());
    chooseAnotherRedundantSuperNode();

    if (write (sd, &ipOfRedundantSuperNode,sizeof(in_addr_t)) <= 0)
    {
        perror ("Eroare la write() de la client.\n");
    }
    close(sd);
}

    void notifyNodeFileNotFound(FileRequest fileRequest);

    void notifyNodeFileFound(FileRequest fileRequest);

    void updateNextSuperNodeToThisNode();

    void getNextRedundantIp();

    void updateNextRedundantIp(in_addr_t nextNodeIp, in_addr_t ipOfNode, in_addr_t ipofRedundantNode);

    void updateNextIpToRedundant();

    void updateNextRedundantIpToRedundant();

    void receiveRequestFromConnectedNode(int sd);

    void receiveRequestFromSuperNode(int sd);
};

#endif //HOST_SUPERNODE_H
