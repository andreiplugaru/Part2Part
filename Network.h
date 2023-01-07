#ifndef HOST_NETWORK_H
#define HOST_NETWORK_H
#include <netinet/in.h>

#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include "Response.h"
#include "Request.h"
#include "Node.h"

class Network {
public:
    static Result makeRequest(in_addr_t ip, in_port_t port, RequestType request, int &sd) {
        struct sockaddr_in server;
        Result result;
        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("Socket error().\n");
            return Failure;
        }
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = ip;
        server.sin_port = port;
        if (connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {//
            // perror("Connect error().\n");
            return Failure;
        }
        if (write(sd, &request, sizeof(RequestType)) <= 0) {
            // perror ("Write error().\n");
            return Failure;
        }
        if (read(sd, &result, sizeof(Result)) <= 0) {
            // perror ("Read error().\n");
            return Failure;
        }
        if (result == Failure)
            return Failure;
        return Success;
    }
    static in_addr_t getIp();
};
void sendFileRequest(int sd, FileRequest fileRequest)
{
    if (write(sd, &fileRequest.id, sizeof(int)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &fileRequest.ipOfTheNodeRequesting, sizeof(in_addr_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &fileRequest.ipOfTheSuperNodeRequesting, sizeof(in_addr_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &fileRequest.ipOfTheNodeWithFile, sizeof(in_addr_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &fileRequest.fileName, sizeof(fileRequest.fileName) * sizeof(char)) <= 0) {
        perror("Eroare la write().\n");
    }
};
FileRequest readFileRequest(int sd)
{
    FileRequest fileRequest;
    if (read(sd, &fileRequest.id, sizeof(int)) <= 0) {
        perror("Eroare la read().\n");
    }
    if (read(sd, &fileRequest.ipOfTheNodeRequesting, sizeof(in_addr_t)) <= 0) {
        perror("Eroare la read().\n");
    }
    if (read(sd, &fileRequest.ipOfTheSuperNodeRequesting, sizeof(in_addr_t)) <= 0) {
        perror("Eroare la read().\n");
    }
    if (read(sd, &fileRequest.ipOfTheNodeWithFile, sizeof(in_addr_t)) <= 0) {
        perror("Eroare la read().\n");
    }
    if (read(sd, &fileRequest.fileName, sizeof(fileRequest.fileName) * sizeof(char)) <= 0) {
        perror("Eroare la read().\n");
    }
    return fileRequest;
};

void sendNode(int sd, Node node) {
    if (write(sd, &node.ip, sizeof(in_addr_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &node.port, sizeof(in_port_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &node.portSuperNode, sizeof(in_port_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &node.portSuperNode, sizeof(in_port_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &node.ipSuperNode, sizeof(in_addr_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &node.shouldBeRedundantSuperNode, sizeof(bool)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &node.isSuperNode, sizeof(bool)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &node.isFirstNode, sizeof(bool)) <= 0) {
        perror("Eroare la write().\n");
    }
};
Node readNode(int sd) {
    Node node;
    if (read(sd, &node.ip, sizeof(in_addr_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (read(sd, &node.port, sizeof(in_port_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (read(sd, &node.portSuperNode, sizeof(in_port_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (read(sd, &node.portSuperNode, sizeof(in_port_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (read(sd, &node.ipSuperNode, sizeof(in_addr_t)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (read(sd, &node.shouldBeRedundantSuperNode, sizeof(bool)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (read(sd, &node.isSuperNode, sizeof(bool)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (read(sd, &node.isFirstNode, sizeof(bool)) <= 0) {
        perror("Eroare la write().\n");
    }
    return node;
};
void sendAcceptSuperNodeResponse(int sd, AcceptSuperNodeResponse acceptSuperNodeResponse)
{
    if (write(sd, &acceptSuperNodeResponse.sd, sizeof(int)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &acceptSuperNodeResponse.result, sizeof(Result)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (write(sd, &acceptSuperNodeResponse.shouldBeRedundantSuperNode, sizeof(bool)) <= 0) {
        perror("Eroare la write().\n");
    }
}
AcceptSuperNodeResponse readAcceptSuperNodeResponse(int sd)
{
    AcceptSuperNodeResponse acceptSuperNodeResponse;
    if (read(sd, &acceptSuperNodeResponse.sd, sizeof(int)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (read(sd, &acceptSuperNodeResponse.result, sizeof(Result)) <= 0) {
        perror("Eroare la write().\n");
    }
    if (read(sd, &acceptSuperNodeResponse.shouldBeRedundantSuperNode, sizeof(bool)) <= 0) {
        perror("Eroare la write().\n");
    }
    return acceptSuperNodeResponse;
}
#endif //HOST_NETWORK_H
