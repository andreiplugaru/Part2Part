#include "Network.h"
#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

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
in_addr_t Network::getIp()
{
    /*struct addrinfo hints = {0}, *addrs;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    int rval = getaddrinfo("whatismyip.akamai.com", "80", &hints, &addrs);
    if (rval != 0) {
        fprintf(stderr,"getaddrinfo failed: %s\n", gai_strerror(rval));
        return 1;
    }

    int s = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
    if (s == -1) {
        perror("socket failed");
        return 1;
    }
    rval = connect(s, addrs->ai_addr, addrs->ai_addrlen);
    if (rval == -1) {
        perror("connect failed");
        return 1;
    }

    char cmd[] = "GET / HTTP/1.1\nHost: whatismyip.akamai.com\n\n";
    rval = send(s, cmd, strlen(cmd), 0);
    if (rval == -1) {
        perror("send failed");
        return 1;
    }

    char buf[1000] = {0};
    rval = recv(s, buf, sizeof(buf), 0);
    if (rval == -1) {
        perror("recv failed");
        return 1;
    }
   // printf("response: %s\n", buf);

    char *start = buf, *end;
    end = strchr(start, '\n');
    if (!strncmp(start, "HTTP/1.1 200 OK", end - start - 1)) {
        while (!(end[1] == '\r' && end[2] == '\n')) {
            start = end + 2;
            end = strchr(start, '\n');
        }
        start = end + 3;
        end = strchr(start, '\n');
        if (end) *end = 0;
      //  printf("my IP: %s\n", start);
    } else {
      //  printf("request failed\n");
    }

    close(s);
    freeaddrinfo(addrs);
    struct sockaddr_in sa;

// store this IP address in sa:
    inet_pton(AF_INET, start, &(sa.sin_addr));
      return sa.sin_addr.s_addr;*/

    const char* google_dns_server = "8.8.8.8";
    int dns_port = 53;

    struct sockaddr_in serv;

    int sock = socket ( AF_INET, SOCK_DGRAM, 0);

    //Socket could not be created
    if(sock < 0)
    {
        perror("Socket error");
    }

    memset( &serv, 0, sizeof(serv) );
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr( google_dns_server );
    serv.sin_port = htons( dns_port );
    int err = connect( sock , (const struct sockaddr*) &serv , sizeof(serv) );
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr*) &name, &namelen);
    close(sock);
    return name.sin_addr.s_addr;
}
