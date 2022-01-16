#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

#include "mnet.h"

#define BACKLOG 5

typedef struct hostent HostEntry;
typedef struct sockaddr_in SocketAddress;

static int initMultiServer(int port);
static void acceptClients(int s_waiting, Clients *clients);

void setupMultiServer(int port, Clients *clients)
{
  int s_waiting;
  s_waiting = initMultiServer(port);
  acceptClients(s_waiting, clients);
}

void closeAllClients(Clients *clients)
{
  int i;
  for (i = 0; i < clients->num; i++)
    close(clients->fd[i]);
  clients->num = 0;
}

static int initMultiServer(int port)
{
  HostEntry *serverInfo;
  SocketAddress serverAddress;
  char serverName[LEN_HOSTNAME];
  int s_waiting;

  gethostname(serverName, LEN_HOSTNAME);
  serverInfo = gethostbyname(serverName);

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

  if (bind(s_waiting, &serverAddress, sizeof(SocketAddress)) == -1)
  {
    fprintf(stderr, "cannot bind.\n");
    exit(1);
  }
  fprintf(stderr, "successfully bound, now listen.\n");

  if (listen(s_waiting, BACKLOG) == -1)
  {
    fprintf(stderr, "cannot listen.\n");
    exit(1);
  }
  fprintf(stderr, "listen ok, now ready to accept.\n");

  return s_waiting;
}

static void acceptClients(int s_waiting, Clients *clients)
{
  fd_set fdset;
  fd_set arrived;
  int fdsetWidth;

  FD_ZERO(&fdset);
  FD_SET(0, &fdset);
  FD_SET(s_waiting, &fdset);
  fdsetWidth = s_waiting + 1;

  bzero(clients, sizeof(Clients));

  while (1)
  {
    arrived = fdset;
    select(fdsetWidth, &arrived, NULL, NULL, NULL);

    if (FD_ISSET(0, &arrived))
    {
      fprintf(stdout, "accept end\n");
      getchar();
      close(s_waiting);
      break;
    }

    if (FD_ISSET(s_waiting, &arrived))
    {
      SocketAddress clientAddress;
      int lengthAddress;
      HostEntry *clientInfo;
      int no = clients->num;

      lengthAddress = sizeof(SocketAddress);
      clients->fd[no] = accept(s_waiting, &clientAddress, &lengthAddress);

      clientInfo = gethostbyaddr((char *)&clientAddress.sin_addr, sizeof(struct in_addr), AF_INET);
      strcpy(clients->name[no], clientInfo->h_name);
      fprintf(stderr, "%s is connected.\n", clients->name[no]);

      clients->num++;

      if (clients->num == MAX_CLIENTS)
        break;
    }
  }

  fprintf(stderr, "accept ok, now ready to conversation.\n");
}
