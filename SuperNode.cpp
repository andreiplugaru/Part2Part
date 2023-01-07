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
        if (write(sd, &fileRequest, sizeof(FileRequest)) <= 0) {
            perror("Eroare la write().\n");
        }
        close(sd);
    }
}

void SuperNode::notifyNodeFileNotFound(FileRequest fileRequest) {
    int sd;
    Result result = Network::makeRequest(fileRequest.ipOfTheNodeRequesting, htons(atoi("2908")), NodeFileNotFound, sd);
    if (result == Success) {
        if (write(sd, &fileRequest, sizeof(FileRequest)) <= 0) {
            perror("Eroare la write().\n");
        }
        close(sd);
    }
}
void SuperNode::notifyNodeFileFound(FileRequest fileRequest) {
    int sd;
    Result result = Network::makeRequest(fileRequest.ipOfTheNodeRequesting, htons(atoi("2908")), NodeFileFound, sd);
    if (result == Success) {
        if (write(sd, &fileRequest, sizeof(FileRequest)) <= 0) {
            perror("Eroare la write().\n");
        }
        close(sd);
    }
}