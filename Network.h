#ifndef HOST_NETWORK_H
#define HOST_NETWORK_H
#include <netinet/in.h>

#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>
#include "Response.h"
#include "Request.h"
int check_host_name(int hostname);
int check_host_entry(struct hostent * hostentry);
int IP_formatter(char *IPbuffer);
Result makeRequest(in_addr_t ip, in_port_t port, RequestType request, int& sd)
{
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
    if (write (sd, &request,sizeof(RequestType)) <= 0)
    {
       // perror ("Write error().\n");
        return Failure;
    }
    if (read (sd, &result,sizeof(Result)) <= 0)
    {
       // perror ("Read error().\n");
        return Failure;
    }
    if(result == Failure)
        return Failure;
    return Success;
}
in_addr_t getIp();
#endif //HOST_NETWORK_H
