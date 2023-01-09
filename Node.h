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
#include "Response.h"
#include "Request.h"
class Node {
public:
    in_addr_t ip;
    in_port_t port;
    in_port_t portSuperNode;
    in_addr_t ipSuperNode;
    bool shouldBeRedundantSuperNode;
    bool isSuperNode = false;
    bool isFirstNode = false;
    std::unordered_map<in_addr_t, int>* scannedSuperNodes;
    std::unordered_map<in_addr_t, in_addr_t> superNodesPrevs;
    std::unordered_map<in_addr_t, in_addr_t> redundantIps;

    Result disconnectFromHost(in_addr_t Ip, in_port_t port);
    NextSuperNodeResponse makeNewSuperNode();
    Result requestInfoFromSuperNode(in_addr_t Ip, in_port_t port);
    Result connectToSuperNode();
    bool hasAvailableSuperNodes();
    void addFiles(std::string fileName);
    void initDB();
    void insertFile(std::string fileName, std::string type, int size);
    Result checkFileExists(FileRequest fileRequest);
    Result sendFile(FileRequest fileRequest);
    Result sendFileToRequestingSuperNode(FileRequest fileRequest);
    void initiateFileTransferRequest(FileRequest fileRequest);
   // void initiateFileTransferSend(FileRequest fileRequest);

    virtual void test(){};
    Node();

    void searchFile(std::string fileName);

    void initiateFileTransferSend(int sd, FileRequest fileRequest);
    void showSharedFiles();

    std::pair<in_addr_t, int> getMaxSuperNode();
};


#endif //HOST_NODE_H
