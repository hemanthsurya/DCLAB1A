#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <calcLib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#ifndef DEBUG
#define DEBUG 0
#endif
#if DEBUG
#define DBG(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#else
#define DBG(fmt, ...) do {} while(0)
#endif

static int readmsg(int sfd, char *buf, size_t buflen) {
    size_t i = 0;
    char s;
    ssize_t n;
    while (i < buflen -1) {
        n = recv(sfd, &s, 1, 0);
        if(n > 0 ) {
            DBG("Data Received: %c", s);
        }else{
            DBG("Error occurred while receiving");
        }
        buf[i++] =   s;
    }
    buf[i] = '\0';
    return (int)i;
}

int main(int argc, char *argv[]){
    if(argc != 2) {
        printf("Usage: %s <host:port>\n", argv[0]);
        return 1;
    }
    
    char delim[]=":";
    char *Desthost=strtok(argv[1],delim);
    char *Destport=strtok(NULL,delim);
    if(!Desthost || !Destport){
        printf("Invalid usage\n");
        return 1;
    }

    int port=atoi(Destport);
#ifdef DEBUG 
  printf("Host %s, and port %d.\n",Desthost,port);
#endif

struct addrinfo hints;
struct addrinfo *res;
struct addrinfo *rp;
memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;
char buf[20];
snprintf(buf, sizeof(buf), "%d", port);
int err = getaddrinfo(Desthost, buf, &hints, &res);
    if (err != 0) {
        printf("ERROR: RESOLVE ISSUE\n");
        return 1;
    }

    int sockfd = -1;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) continue;
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(sockfd);
        sockfd = -1;
    }
    freeaddrinfo(res);

    if(sockfd == -1) {
        printf("ERROR: CANT CONNECT TO %s\n", Desthost);
        return 1;
    }
    DBG("Connected to %s:%d", Desthost, port);

    char msg[128];
    int supported = 0;
    while (readmsg(sockfd, msg, 128) > 0) {
        DBG("Server %s", msg);
        if(strcmp(msg, "TEXT TCP 1.0") == 0)
            supported = 1;
        if(msg[0] == '\0')
            DBG("End of line");
    }

    if(!supported){
        printf("ERROR: MISMATCH PROTOCOL\n");
        close(sockfd);
        return 1;
    } else {
        send(sockfd, "OK\n", 3, 0);
    }

    if(readmsg(sockfd, msg, 128) <= 0) {
        printf("ERROR: NO INPUT RECEIVED\n");
        close(sockfd);
        return 1;
    }

}