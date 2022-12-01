#ifndef HOST_NETWORK_H
#define HOST_NETWORK_H
#include <netinet/in.h>

#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string>

struct Node{
    in_port_t ip;
    in_port_t port;
    bool isSuperNode = false;
    bool isFirstNode = false;
};
int check_host_name(int hostname);
int check_host_entry(struct hostent * hostentry);
int IP_formatter(char *IPbuffer);
std::string getIp();
#endif //HOST_NETWORK_H
