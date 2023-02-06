#include <cstring>
#include "SuperNode.h"
#include "Request.h"

void SuperNode::receiveRequestFromConnectedNode(int sd) {
    FileRequest fileRequest;
    Network::receive(sd, fileRequest);
    fileRequest.ipOfTheSuperNodeRequesting = Network::getIp();
    Result result1 = checkFileExists(fileRequest);
    if(result1 == Success) {
        Result result1 = Success;
        if (write(sd, &result1, sizeof(Result)) <= 0) {
            perror("Eroare la write().\n");
        }
        fileRequest.ipOfTheNodeWithFile = Network::getIp();
        sendFileToRequestingSuperNode(fileRequest);
    }
    else {
        bool found = false;
        for (int i = 0; i < connectedNodes.size(); i++) {

            if (connectedNodes[i]->ip != fileRequest.ipOfTheNodeRequesting) {
                int sd2;
                Result resultRequestFile = Network::makeRequest(connectedNodes[i]->ip,
                                                                htons(atoi("2908")), CheckFileExists,
                                                                sd2);
                if (resultRequestFile == Success) {
                    Network::send(sd2, fileRequest);
                    Result result1;
                    if (read(sd2, &result1, sizeof(Result)) <= 0) {
                        perror("Eroare la write().\n");
                    }
                    Network::receive(sd2, fileRequest);

                    close(sd2);
                    if (fileRequest.ipOfTheNodeWithFile != 0) {
                        found = true;
                        fileRequest.ipOfTheNodeWithFile = connectedNodes[i]->ip;
                        sendFileToRequestingSuperNode(fileRequest);
                        break;
                    }
                }
            }
        }
        if (!found) {
            Result result1 = SearchInOtherSuperNodes;
            if (write(sd, &result1, sizeof(Result)) <= 0) {
                perror("Eroare la write().\n");
            }
            close(sd);
            int sd2;
            Result resultRequestFile = Network::makeRequest(nextIpSuperNode,
                                                            htons(atoi("2908")), RequestFileFromSuperNode,
                                                            sd2);
            if (resultRequestFile == Success) {
                Network::send(sd, fileRequest);

            }
            close(sd2);
        }
    }
}
void SuperNode::receiveRequestFromSuperNode(int sd) {
    FileRequest fileRequest;
    Network::receive(sd, fileRequest);
    if(!hasRequest(fileRequest)) {
        Result result1 = checkFileExists(fileRequest);
        if (result1 == Success) {
            fileRequest.ipOfTheNodeWithFile = Network::getIp();
            sendFileToRequestingSuperNode(fileRequest);
        } else {
            bool found = false;
            for (int i = 0; i < connectedNodes.size(); i++) {
                if (connectedNodes[i]->ip != fileRequest.ipOfTheNodeRequesting) {
                    int sd2;
                    Result resultRequestFile = Network::makeRequest(
                            connectedNodes[i]->ip,
                            htons(atoi("2908")), CheckFileExists,
                            sd2);
                    if (resultRequestFile == Success) {
                        Network::send(sd2, fileRequest);

                        Result result1;
                        if (read(sd2, &result1, sizeof(Result)) <= 0) {
                            perror("Eroare la write().\n");
                        }
                        Network::receive(sd2, fileRequest);

                        close(sd2);
                        if (fileRequest.ipOfTheNodeWithFile != 0) {
                            printf("file found in this super node\n");
                            found = true;
                            fileRequest.ipOfTheNodeWithFile = connectedNodes[i]->ip;
                            sendFileToRequestingSuperNode(fileRequest);
                            break;
                        }
                    }
                }
            }

            if (!found) {
                std::lock_guard<std::mutex> lock(pendingRequests_mutex);
                pendingRequests.push_back(fileRequest);
                int sd2;
                Result resultRequestFile = Network::makeRequest(nextIpSuperNode,
                                                                htons(atoi("2908")), RequestFileFromSuperNode,
                                                                sd2);
                if (resultRequestFile == Success) {
                    Network::send(sd2, fileRequest);
                }
                close(sd2);
            }
        }
    }

    else
    {
        notifySuperNodeFileNotFound(fileRequest);
    }
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
    Result result = Network::makeRequest(fileRequest.ipOfTheNodeRequesting, htons(atoi("2908")), NodeFileFound, sd);
    if (result == Success) {
        Network::send(sd, fileRequest);

        close(sd);
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
void SuperNode::removeConnectedNodesFromSuperNode(int id)
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
void SuperNode::disconnect(in_addr_t ip)
{
    int id = -1;
    for (int i = 0; i < connectedNodes.size(); i++) {
        if (connectedNodes[i]->ip == ip) {
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
void SuperNode::chooseAnotherRedundantSuperNode()
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
         result = Network::makeRequest(connectedNodes[i]->ip, htons(atoi("2908")), ChooseAsRedunantSuperNode, sd);
        if (result == Success) {
            ipOfRedundantSuperNode = connectedNodes[i]->ip;
        }
        i++;
    }
    close(sd);
}
Result SuperNode::acceptNewNode(in_addr_t ip, in_port_t port, int sd) {
    if (this->connectedNodes.size() < MAX_CLIENTS_PER_SUPERNODE) {
        Node *newNode = new Node();
        newNode->ip = ip;
        newNode->port = port;
        std::lock_guard<std::mutex> lock(connectedNodes_mutex);
        connectedNodes.push_back(newNode);
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip, ipStr, INET_ADDRSTRLEN);
        int result = Success;

        AcceptSuperNodeResponse response;
        response.result = Success;
        response.shouldBeRedundantSuperNode = connectedNodes.size() == 1;

        if(this->ipOfRedundantSuperNode != 0)
        {
            int sd1;
            Result result1 = Network::makeRequest(ipOfRedundantSuperNode, htons(atoi("2908")), RequestType::SendNewNodeToRedundantSuperNode, sd1);
            if(result1 == Success)
            {
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
void SuperNode::sendConnectedNodes(int sd)
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
void* SuperNode::ping()
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
                Result resultRedundant = Network::makeRequest(ipOfNextRedundantSuperNode, htons(atoi("2908")),
                                                              BecomeSuperNode, sd);
                if (resultRedundant == Success) {
                    nextIpSuperNode = ipOfNextRedundantSuperNode;
                    if (read(sd, &ipOfNextRedundantSuperNode, sizeof(in_addr_t)) <= 0) {
                        perror("Eroare la write().\n");
                    }
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
void SuperNode::getConnectedNodesFromSuperNode()
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
void SuperNode::transformToNonRedundantSuperNode(int sd)
{
    isRedundantSuperNode = false;
    if(connectedNodes.size() > 0)
        connectedNodes.erase(connectedNodes.begin());
    chooseAnotherRedundantSuperNode();
    if (write (sd, &ipOfRedundantSuperNode,sizeof(in_addr_t)) <= 0)
    {
        perror ("Eroare la write() de la client.\n");
    }
    close(sd);
}