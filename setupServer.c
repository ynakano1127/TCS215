#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "snet.h"

typedef struct hostent HostEntry;
typedef struct sockaddr_in SocketAddress;

#define HOST_LEN 64

int setupServer(int port)
{
  HostEntry *serverInfo;
  SocketAddress serverAddress;
  char serverName[HOST_LEN];
  int s_waiting;
  int s;

  gethostname(serverName, HOST_LEN);

  if ((serverInfo = gethostbyname(serverName)) == NULL)
  {
    fprintf(stderr, "bad hostname!\n");
    exit(1);
  }

  bzero((char *)&serverAddress, sizeof(SocketAddress));
  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(port);
  bcopy(serverInfo->h_addr,
        (char *)&serverAddress.sin_addr, serverInfo->h_length);

  if ((s_waiting = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    fprintf(stderr, "socket allocation failed.\n");
    exit(1);
  }

  if (bind(s_waiting, (struct sockaddr*)&serverAddress, sizeof(SocketAddress)) == -1)
  {
    fprintf(stderr, "cannot bind.\n");
    exit(1);
  }
  fprintf(stderr, "successfully bound, now listen.\n");

  listen(s_waiting, 1);

  s = accept(s_waiting, NULL, NULL);
  close(s_waiting);

  return s;
}
