#ifndef HOST_SUPERNODE_H
#define HOST_SUPERNODE_H

#include "Node.h"
class SuperNode : public Node{
public:
    in_addr_t nextIpSuperNode;
    in_port_t nextPortSuperNode;
    in_addr_t prevIpSuperNode;
    in_port_t prevPortSuperNode;
    std::vector<Node*> connectedNodes;

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
Result acceptNewNode(in_addr_t ip, in_port_t port, int sd)
{
    if(this->connectedNodes.size() < MAX_CLIENTS_PER_SUPERNODE) {
        Node *newNode = new Node();
        newNode->ip = ip;
        newNode->port = port;
        connectedNodes.push_back(newNode);
        char *ipStr;
        inet_ntop(AF_INET, (void *)ip, ipStr, INET_ADDRSTRLEN);
        int result = Success;
        if (write(sd, &result, sizeof(int)) <= 0) {
            perror("Eroare la write().\n");
        }
        printf("Node with ip %s connected!\n", ip);
    }
    close(sd);
    return Success;
}
Result rejectNewNode(in_addr_t ip, in_port_t port, int sd)
{
    char *ipStr;
    inet_ntop(AF_INET, (void *)ip, ipStr, INET_ADDRSTRLEN);
    int result = Reject;
    if (write(sd, &result, sizeof(int)) <= 0) {
        perror("Eroare la write().\n");
    }
    Node nextSuperNode;
    nextSuperNode.ip = this->nextIpSuperNode;
    nextSuperNode.port = this->nextPortSuperNode;
    if (write(sd, &nextSuperNode, sizeof(Node)) <= 0) {
        perror("Eroare la write().\n");
    }
    close(sd);
}
Result rejectNewNodeNotSuper(in_addr_t ip, in_port_t port, int sd)
{
    char *ipStr;
    inet_ntop(AF_INET, (void *)ip, ipStr, INET_ADDRSTRLEN);
    int result = Reject;
    if (write(sd, &result, sizeof(int)) <= 0) {
        perror("Eroare la write().\n");
    }
    Node nextSuperNode;
    nextSuperNode.ip = this->nextIpSuperNode;
    nextSuperNode.port = this->nextPortSuperNode;
    if (write(sd, &nextSuperNode, sizeof(Node)) <= 0) {
        perror("Eroare la write().\n");
    }
    close(sd);
}
};
#endif //HOST_SUPERNODE_H
