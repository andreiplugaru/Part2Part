#include <cstring>
#include "SuperNode.h"
#include "Request.h"

Result SuperNode::receiveRequestFromConnectedNode(FileRequest fileRequest) {

    }

Result SuperNode::receiveRequestFromSuperNode(FileRequest fileRequest) {

    return Success;
}

bool SuperNode::hasRequest(FileRequest fileRequest) {
    for (int i = 0; i < pendingRequests.size(); ++i) {
        if(strcmp(pendingRequests[i].fileName,fileRequest.fileName) == 0) {
            std::lock_guard<std::mutex> lock(pendingRequests_mutex);

            pendingRequests.erase(pendingRequests.begin() + i);
            return true;
        }
    }
    return false;
}

void SuperNode::notifySuperNodeFileNotFound(FileRequest fileRequest) {
    int sd;
    Result result = Network::makeRequest(fileRequest.ipOfTheSuperNodeRequesting, htons(atoi("2908")), SuperNodeFileNotFound, sd);
    if (result == Success) {
        Network::send(sd, fileRequest);

        close(sd);
    }
}

void SuperNode::notifyNodeFileNotFound(FileRequest fileRequest) {
    int sd;
    Result result = Network::makeRequest(fileRequest.ipOfTheNodeRequesting, htons(atoi("2908")), NodeFileNotFound, sd);
    if (result == Success) {
        Network::send(sd, fileRequest);

        close(sd);
    }
}
void SuperNode::notifyNodeFileFound(FileRequest fileRequest) {
    int sd;
    printf("\nnotifyNodeFileFound\n");
    Result result = Network::makeRequest(fileRequest.ipOfTheNodeRequesting, htons(atoi("2908")), NodeFileFound, sd);
    if (result == Success) {
        Network::send(sd, fileRequest);

        close(sd);
    }
}
void SuperNode::updateNextSuperNodeToThisNode()
{
    int sd;
    Result result = Network::makeRequest(nextIpSuperNode, htons(atoi("2908")), UpdateNextNodeNeighbour, sd);
    if (result == Success) {
        if (write (sd, &ip,sizeof(in_addr_t)) <= 0)
        {
            perror ("Eroare la write() de la client.\n");
        }
    }
}

void SuperNode::getNextIp() {
    int sd;
    Result result = Network::makeRequest(ipSuperNode, htons(atoi("2908")), RequestType::GetNextSuperNode, sd);
    if(result == Success)
    {
        if (read (sd,&nextIpSuperNode,sizeof(in_addr_t)) <= 0)
        {
            perror ("Read error().\n");
        }
    }
    close(sd);
}
void SuperNode::getNextRedundantIp() {
    int sd;
    Result result = Network::makeRequest(ipSuperNode, htons(atoi("2908")), RequestType::GetNextRedundantSuperNode, sd);
    if(result == Success)
    {
        if (read (sd,&ipOfNextRedundantSuperNode,sizeof(in_addr_t)) <= 0)
        {
            perror ("Read error().\n");
        }
    }
    close(sd);
}
void SuperNode::updateNextRedundantIp(in_addr_t nextNodeIp,  in_addr_t ipOfNode,in_addr_t ipofRedundantNode) {
    int sd;
    Result result = Network::makeRequest(nextNodeIp, htons(atoi("2908")), RequestType::UpdateNextRedundant, sd);
    if(result == Success)
    {
        if (write (sd,&ipOfNode,sizeof(in_addr_t)) <= 0)
        {
            perror ("Read error().\n");
        }
        if (write (sd,&ipofRedundantNode,sizeof(in_addr_t)) <= 0)
        {
            perror ("Read error().\n");
        }
    }
    close(sd);
}
/*void SuperNode::updateNextIp(in_addr_t nextNodeIp,  in_addr_t ipOfNode,in_addr_t ipofRedundantNode) {
    int sd;
    Result result = Network::makeRequest(nextNodeIp, htons(atoi("2908")), RequestType::UpdateNextRedundant, sd);
    if(result == Success)
    {
        if (write (sd,&ipOfNode,sizeof(in_addr_t)) <= 0)
        {
            perror ("Read error().\n");
        }
        if (write (sd,&ipofRedundantNode,sizeof(in_addr_t)) <= 0)
        {
            perror ("Read error().\n");
        }
    }
    close(sd);
}*/
void SuperNode::updateNextIpToRedundant() {
    if(ipOfRedundantSuperNode != 0) {
        int sd;
        Result result = Network::makeRequest(ipOfRedundantSuperNode, htons(atoi("2908")), RequestType::UpdateNextIpToRedundant, sd);
        if (result == Success) {
            if (write (sd, &nextIpSuperNode,sizeof(in_addr_t)) <= 0)
            {
                perror ("Eroare la write() de la client.\n");
            }
        }
        close(sd);
    }
}
void SuperNode::updateNextRedundantIpToRedundant() {
    if(ipOfRedundantSuperNode != 0) {
        int sd;
        Result result = Network::makeRequest(ipOfRedundantSuperNode, htons(atoi("2908")), RequestType::UpdateNextRedundantIpToRedundant, sd);
        if (result == Success) {
            if (write (sd, &ipOfNextRedundantSuperNode,sizeof(in_addr_t)) <= 0)
            {
                perror ("Eroare la write() de la client.\n");
            }
        }
        close(sd);
    }
}