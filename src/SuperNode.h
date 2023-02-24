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
    }    void notifySuperNodeFileNotFound(FileRequest fileRequest);
    SuperNode(){
            ipOfRedundantSuperNode = 0;
    }
    void removeConnectedNodesFromSuperNode(int id);
    void disconnect(in_addr_t ip);


    void getNextIp();
    void chooseAnotherRedundantSuperNode();

    Result acceptNewNode(in_addr_t ip, in_port_t port, int sd);
    void sendConnectedNodes(int sd);
    void* ping();

/*This function is used if the current node is a redundant super node*/
    void getConnectedNodesFromSuperNode();
    void transformToNonRedundantSuperNode(int sd);
    void notifyNodeFileNotFound(FileRequest fileRequest);
    void notifyNodeFileFound(FileRequest fileRequest);
    void updateNextSuperNodeToThisNode();
    void getNextRedundantIp();
    void updateNextRedundantIp(in_addr_t nextNodeIp, in_addr_t ipOfNode, in_addr_t ipofRedundantNode);
    void updateNextIpToRedundant();
    void updateNextRedundantIpToRedundant();
    void receiveRequestFromConnectedNode(int sd);
    void receiveRequestFromSuperNode(int sd);
    bool hasRequest(FileRequest fileRequest);
};

#endif //HOST_SUPERNODE_H
