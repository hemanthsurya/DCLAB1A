#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <calcLib.h>

#ifndef DEBUG
#define DEBUG 0
#endif
#if DEBUG
#define DBG(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#else
#define DBG(fmt, ...) do {} while(0)
#endif


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

  /* Do magic */
  int port=atoi(Destport);
#ifdef DEBUG 
  printf("Host %s, and port %d.\n",Desthost,port);
#endif

  
}