#include "Network.h"
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>


in_addr_t Network::getIp() {
   /* const char *google_dns_server = "8.8.8.8";
    int dns_port = 53;
    struct sockaddr_in serv;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket error");
    }
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(google_dns_server);
    serv.sin_port = htons(dns_port);
    int err = connect(sock, (const struct sockaddr *) &serv, sizeof(serv));
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr *) &name, &namelen);
    close(sock);

    return name.sin_addr.s_addr;*/

    FILE *curl;
    if((curl = popen("curl https://icanhazip.com/", "r")) == NULL){
        printf("ERROR: Failed to run curl.\n");
        return 1;
    }

    char* ip = (char*)malloc(99);
    fgets(ip, 99, curl);

    in_addr_t ip2 = inet_addr(ip);

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip2, ipStr, INET_ADDRSTRLEN);
    printf("ipSTr = %s\n",ipStr);
    printf("ip = %s\n",ip);
    return ip2;
   // return ifAddrStruct->ifa_addr->sa
}
