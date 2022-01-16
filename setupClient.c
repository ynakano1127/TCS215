#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "snet.h"

typedef struct hostent HostEntry;
typedef struct sockaddr_in SocketAddress;

int setupClient(char *serverName, int port)
{
  HostEntry *serverInfo;
  SocketAddress serverAddress;
  int s;

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

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    fprintf(stderr, "socket allocation failed.\n");
    exit(1);
  }

  if (connect(s, (struct sockaddr*)&serverAddress, sizeof(SocketAddress)) == -1)
  {
    fprintf(stderr, "cannot connect.\n");
    exit(1);
  }
  fprintf(stderr, "successfully connected, now begin.\n");

  return s;
}
