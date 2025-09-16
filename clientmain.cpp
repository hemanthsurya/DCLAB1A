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
#include <sys/time.h>

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
        if(n <= 0 )
            break;
        else if (s == '\n')
            break;
        buf[i++] = s;
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
    printf("Host %s, and port %d.\n",Desthost,port);


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

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    char msg[128];
    int supported = 0;
    while (readmsg(sockfd, msg, sizeof(msg)) > 0) {
        DBG("Server %s", msg);
        if(strcmp(msg, "TEXT TCP 1.0") != 0) {
            printf("ERROR\n");
            close(sockfd);
            return 1;
        } else {
            supported = 1;
        }         
        if(msg[0] == '\0'){
            DBG("End of line");
            break;
        }
    }

    if(!supported){
        printf("ERROR: MISSMATCH PROTOCOL\n");
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
    
    printf("ASSIGNMENT: %s\n", msg);
    
    char operation[16];
    char value1[16];
    char value2[16];
    if(sscanf(msg,"%s %s %s", operation, value1, value2) != 3) {
        printf("ERROR: During reading values %s\n", msg);
        close(sockfd);
        return 1;
    }

    int ires = 0;
    float fres = 0.0;
    int isint = 0;
    int isfloat = 0.0;
    if (strcmp(operation, "add") == 0 || strcmp(operation, "sub") == 0 || strcmp(operation, "mul") == 0 || strcmp(operation, "div") == 0){
        isint = 1;
        int a = atoi(value1);
        int b = atoi(value2);
        if (strcmp(operation, "add") == 0)
            ires = a + b;
        else if(strcmp(operation, "sub") == 0)
            ires = a - b;
        else if(strcmp(operation, "mul") == 0)
            ires = a * b;
        else if(strcmp(operation, "div") == 0) {
            if(b != 0)
                ires = a/b;
            else
                ires = 0;
        }
    } else if(strcmp(operation, "fadd") == 0 || strcmp(operation, "fsub") == 0 || strcmp(operation, "fmul") == 0 || strcmp(operation, "fdiv") == 0){
        isfloat = 1;
        float a = atof(value1);
        float b = atof(value2);
        if (strcmp(operation, "fadd") == 0)
            fres = a + b;
        else if(strcmp(operation, "fsub") == 0)
            fres = a - b;
        else if(strcmp(operation, "fmul") == 0)
            fres = a * b;
        else if(strcmp(operation, "fdiv") == 0) {
            if(b != 0.0)
                fres = a/b;
            else
                fres = 0.0;
        }
    } else {
        printf("ERROR: UNKOWN OPERATION\n");
        close(sockfd);
        return 1;
    }
    if(isint)
        DBG("Result: %d", ires);
    else if(isfloat)
        DBG("Result: %8.8g", fres);

    char result[16];
    if (isint)
        snprintf(result, 16, "%d\n", ires);
    else if(isfloat)
        snprintf(result, 16, "%8.8g\n", fres);

    send(sockfd, result, strlen(result), 0);

    if(readmsg(sockfd, msg, sizeof(msg)) > 0) {
        printf("%s (myresult=%s)\n", msg, result);
    }
    close(sockfd);
    return 0;

}