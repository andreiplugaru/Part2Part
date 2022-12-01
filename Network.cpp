#include "Network.h"

int check_host_name(int hostname) { //This function returns host name for local computer
    if (hostname == -1) {
        return -1;
    }
}
int check_host_entry(struct hostent * hostentry) { //find host info from host name
    if (hostentry == NULL){
       return -1;
    }
}
int IP_formatter(char *IPbuffer) { //convert IP string to dotted decimal format
    if (NULL == IPbuffer) {
        perror("inet_ntoa");
        exit(1);
    }
}
std::string getIp()
{
    char host[256];
    char *IP;
    struct hostent *host_entry;
    int hostname;
    hostname = gethostname(host, sizeof(host)); //find the host name
    check_host_name(hostname);
    host_entry = gethostbyname(host); //find host information
    check_host_entry(host_entry);
    IP = inet_ntoa(*((struct in_addr*) host_entry->h_addr_list[0])); //Convert into IP string
    return std::string(IP);
}