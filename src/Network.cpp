#include "Network.h"
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>


in_addr_t Network::getIp() {
       FILE *curl;
    if((curl = popen("curl https://icanhazip.com/ -s","r")) == NULL){
        printf("ERROR: Failed to run curl.\n");
        return 1;
    }

    char* ip = (char*)malloc(99);
    fgets(ip, 99, curl);

    in_addr_t ip2 = inet_addr(ip);

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip2, ipStr, INET_ADDRSTRLEN);
    return ip2;
}
