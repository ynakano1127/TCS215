#define MAX_CLIENTS 128
#define LEN_HOSTNAME 64

typedef struct
{
  int num;
  int fd[MAX_CLIENTS];
  char name[MAX_CLIENTS][LEN_HOSTNAME];
} Clients;

void setupMultiServer(int port, Clients *clients);

void closeAllClients(Clients *clients);
