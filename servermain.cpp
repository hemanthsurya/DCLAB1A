#include <calcLib.h>
#include <math.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define DEBUG

static int send(int fd, const char *buf, size_t len) {
  size_t total = 0;
  while (total < len) {
    ssize_t n = send(fd, buf + total, len - total, 0);
    if (n <= 0)
      return -1;
    total += (size_t)n;
  }
  return 0;
}

static int receive(int fd, char *out, size_t maxlen) {
  struct pollfd pfd;
  pfd.fd = fd;
  pfd.events = POLLIN;
  size_t pos = 0;
  char c;
  while (pos < maxlen - 1) {
    int ready = poll(&pfd, 1, 5000);
    if (ready <= 0)
      return -1;
    ssize_t n = recv(fd, &c, 1, 0);
    if (n <= 0)
      return -1;
    if (c == '\n')
      break;
    out[pos++] = c;
  }
  out[pos] = '\0';
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <IP-or-DNS:PORT>\n", argv[0]);
    return 1;
  }

  char delim[] = ":";
  char *Desthost = strtok(argv[1], delim);
  char *Destport = strtok(NULL, delim);
  int port = atoi(Destport);
  if (!Desthost || !Destport) {
    printf("Wrong input arguments\n");
    return 1;
  }

  struct addrinfo hints, *res, *rp;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int err = getaddrinfo(Desthost, Destport, &hints, &res);
  if (err != 0) {
#ifdef DEBUG
    printf("getaddrinfo\n");
#endif
    return 1;
  }
  int listen_fd = -1;
  for (rp = res; rp; rp = rp->ai_next) {
    listen_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (listen_fd == -1)
      continue;
    int i = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
    if (bind(listen_fd, rp->ai_addr, rp->ai_addrlen) == 0)
      break;
    close(listen_fd);
    listen_fd = -1;
  }
  freeaddrinfo(res);

  if (listen_fd == -1) {
#ifdef DEBUG
    printf("ERROR: BIND FAILED");
#endif
    return 1;
  }

  if (listen(listen_fd, 5) < 0) {
#ifdef DEBUG
    printf("ERROR: LISTEN FAILED");
#endif
    close(listen_fd);
    return 1;
  }

  printf("Server listening on %s:%d\n", Desthost, port);

  while (1) {
    struct sockaddr_storage client_addr;
    socklen_t client_len = sizeof(client_addr);
    int connection_fd =
        accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
    if (connection_fd < 0) {
      printf("Connection fd error\n");
      continue;
    }

    if (send(connection_fd, "TEXT TCP 1.0\n\n", 14) < 0) {
      close(connection_fd);
      continue;
    }

    char clientMsg[256];
    if (receive(connection_fd, clientMsg, sizeof(clientMsg)) < 0 ||
        strcmp(clientMsg, "OK") != 0) {
      close(connection_fd);
      continue;
    }

    char *op = randomType();
    int iv1 = randomInt();
    int iv2 = randomInt();
    double fv1 = randomFloat();
    double fv2 = randomFloat();

    int iresult = 0;
    double fresult = 0.0;
    char msg[1450];
    memset(msg, 0, sizeof(msg));

    if (op[0] == 'f') {
      if (strcmp(op, "fadd") == 0)
        fresult = fv1 + fv2;
      else if (strcmp(op, "fsub") == 0)
        fresult = fv1 - fv2;
      else if (strcmp(op, "fmul") == 0)
        fresult = fv1 * fv2;
      else if (strcmp(op, "fdiv") == 0)
        fresult = fv1 / fv2;
      snprintf(msg, sizeof(msg), "%s %8.8g %8.8g\n", op, fv1, fv2);
    } else {
      if (strcmp(op, "add") == 0)
        iresult = iv1 + iv2;
      else if (strcmp(op, "sub") == 0)
        iresult = iv1 - iv2;
      else if (strcmp(op, "mul") == 0)
        iresult = iv1 * iv2;
      else if (strcmp(op, "div") == 0)
        iresult = iv1 / iv2;
      snprintf(msg, sizeof(msg), "%s %d %d\n", op, iv1, iv2);
    }

    if (send(connection_fd, msg, strlen(msg)) < 0) {
      close(connection_fd);
      continue;
    }

    // Step 3: receive client result
    char clientResult[256];
    if (receive(connection_fd, clientResult, sizeof(clientResult)) < 0) {
      send(connection_fd, "ERROR TO\n", 9);
      close(connection_fd);
      continue;
    }

    int correct = 0;
    if (op[0] == 'f') {
      double val = atof(clientResult);
      if (fabs(val - fresult) < 0.001)
        correct = 1;
      else
        correct = 0;
    } else {
      int val = atoi(clientResult);
      correct = (val == iresult);
    }

    if (correct)
      send(connection_fd, "OK\n", 3);
    else
      send(connection_fd, "ERROR\n", 6);

    close(connection_fd);
  }

  close(listen_fd);
  return 0;
}
