//
// Created by andrew01 on 12/4/22.
//

#ifndef HOST_NODE_H
#define HOST_NODE_H
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include <unordered_map>
#include "Defines.h"
class Node {
public:
   // std::string name;
    in_addr_t ip;
    in_port_t port;
    in_port_t portSuperNode;
    in_addr_t ipSuperNode;
    bool shouldBeRedundantSuperNode;
    bool isSuperNode = false;
    bool isFirstNode = false;
    std::unordered_map<in_addr_t, double>* scannedSuperNodes;
    //virtual ~Node() = default;
    Result disconnectFromHost(in_addr_t Ip, in_port_t port);
    NextSuperNodeResponse makeNewSuperNode();
    Result requestInfoFromSuperNode(in_addr_t Ip, in_port_t port);
    Result connectToSuperNode();
    bool hasAvailableSuperNodes();
    void addFiles();
    virtual void test(){};
    Node();
};


#endif //HOST_NODE_H
