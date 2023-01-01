#ifndef HOST_SUPERNODE_H
#define HOST_SUPERNODE_H

#include <pthread.h>
#include "Node.h"
typedef void * (*THREADFUNCPTR)(void *);

class SuperNode : public Node {
public:
    in_addr_t nextIpSuperNode;
    in_port_t nextPortSuperNode;
/*    in_addr_t prevIpSuperNode;
    in_port_t prevPortSuperNode;*/
    std::vector<Node *> connectedNodes;
    bool isRedundantSuperNode;
    in_addr_t ipOfRedundantSuperNode;
    bool isAlone;

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

}
    void removeConnectedNodesFromSuperNode(int id)
    {
        int sd;
        Result result = makeRequest(ipOfRedundantSuperNode, htons(atoi("2908")), RequestType::RemoveNodeFromRedundantSuperNode, sd);
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
            if (connectedNodes[i]->ip ==ip) {
                connectedNodes.erase(connectedNodes.begin() + i);

                if(ipOfRedundantSuperNode == ip) {
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
        if(connectedNodes.size() < 0)
            return;
            int sd;
           Result result =  makeRequest(connectedNodes[0]->ip,htons(atoi("2908")),ChooseAsRedunantSuperNode, sd);
           if(result == Success)
           {
            ipOfRedundantSuperNode = connectedNodes[0]->ip;
           }
        close(sd);
    }
    static Node* makeRedundantSuperNode(Node* currentNode)
    {
        printf("this is redundantnt\n");
        Node *newNode = new SuperNode();
        ((SuperNode *) newNode)->isRedundantSuperNode = true;
        newNode->ipSuperNode = currentNode->ipSuperNode;
        newNode->ip = currentNode->ip;
        ((SuperNode *) newNode)->getConnectedNodesFromSuperNode();
        return newNode;
    }
    Result acceptNewNode(in_addr_t ip, in_port_t port, int sd) {
        if (this->connectedNodes.size() < MAX_CLIENTS_PER_SUPERNODE) {
            Node *newNode = new Node();
            newNode->ip = ip;
            newNode->port = port;
            connectedNodes.push_back(newNode);
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
                Result result1 = makeRequest(ipOfRedundantSuperNode, htons(atoi("2908")), RequestType::SendNewNodeToRedundantSuperNode, sd1);
             //   printf("after   Result result1 = makeRequest(ipOfRedundantSuperNode\n");
                if(result1 == Success)
                {
                    if (write(sd1, &(*connectedNodes[connectedNodes.size() - 1]).ip, sizeof(in_addr_t)) <= 0) {
                        perror("Eroare la write().\n");
                    }
                }
                close(sd1);
            }
            if(response.shouldBeRedundantSuperNode)
                this->ipOfRedundantSuperNode = ip;

            if (write(sd, &response, sizeof(AcceptSuperNodeResponse)) <= 0) {
                perror("Eroare la write().\n");
            }
            printf("Node with ip %s connected!\n", ipStr);
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
         if (write(sd, &connectedNodes[i], sizeof(Node)) <= 0) {
             perror("Eroare la write().\n");
         }
     }
 }

void* ping()
{
    while (true)
    {
        std::vector<in_addr_t> disconnectedIps;
        for (int i = 0; i < connectedNodes.size(); i++) {
            int sd;
            Result check = makeRequest(connectedNodes[i]->ip, htons(atoi("2908")), Ping, sd);
            close(sd);
            if (check == Failure) {
                printf("A node has disconnected!\n");
                disconnectedIps.push_back(connectedNodes[i]->ip);
            }
        }
        for (int i = 0; i < disconnectedIps.size(); i++) {
            disconnect(disconnectedIps[i]);
        }
        sleep(10);
    }
    return NULL;
}
/*This function is used if the current node is a redundant super node*/
void getConnectedNodesFromSuperNode()
{
    int sd;
    Result result = makeRequest(ipSuperNode, htons(atoi("2908")), RequestType::GetConnectedNodes, sd);
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
            if (read(sd, &currentNode, sizeof(Node)) <= 0) {
                perror("Eroare la write().\n");
            }
            currentNode.scannedSuperNodes = new std::unordered_map<in_addr_t, double>();
            connectedNodes.push_back(&currentNode);
        }
    }
    close(sd);
}

/*esult rejectNewNodeNotSuper(in_addr_t ip, in_port_t port, int sd)
{
    char *ipStr;
    inet_ntop(AF_INET, (void *)ip, ipStr, INET_ADDRSTRLEN);
    if (write(sd, &result, sizeof(int)) <= 0) {
        perror("Eroare la write().\n");
    }
    if()
    Node nextSuperNode;
    nextSuperNode.ip = this->nextIpSuperNode;
    nextSuperNode.port = this->nextPortSuperNode;
    if (write(sd, &nextSuperNode, sizeof(Node)) <= 0) {
        perror("Eroare la write().\n");
    }
    close(sd);
}
*/
    };

#endif //HOST_SUPERNODE_H
