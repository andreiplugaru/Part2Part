//
// Created by andrew01 on 12/4/22.
//

#ifndef HOST_NODE_H
#define HOST_NODE_H
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include "Defines.h"
class Node {
public:
    std::string name;
    in_addr_t ip;
    in_port_t port;
    in_port_t portSuperNode;
    in_addr_t ipSuperNode;
    bool isSuperNode = false;
    bool isFirstNode = false;
    virtual ~Node() {}
    Result connectToSuperNode(in_addr_t Ip, in_port_t port);
    Result disconnectFromHost(in_addr_t Ip, in_port_t port);
    void makeNewSuperNode();
};


#endif //HOST_NODE_H
