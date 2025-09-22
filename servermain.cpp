#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <calcLib.h>

#define DEBUG


using namespace std;


int main(int argc, char *argv[]){
  if (argc != 2) {
    printf("Usage: %s <IP-or-DNS:PORT>\n", argv[0]);
    return 1;
  }
  
  char delim[]=":";
  char *Desthost=strtok(argv[1],delim);
  char *Destport=strtok(NULL,delim);
  int port=atoi(Destport);
  if (!Desthost || !Destport) {
    printf("Wrong input arguments\n");
    return 1;
  }

  struct addrinfo hints, *res, *rp;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;

  int err = getaddrinfo(Desthost, Destport, &hints, &res);
  if (err != 0) {
    #ifdef DEBUG 
    printf("getaddrinfo\n");
    #endif
    return 1;
  }
  
#ifdef DEBUG  
  printf("Host %s, and port %d.\n",Desthost,port);
#endif


}
