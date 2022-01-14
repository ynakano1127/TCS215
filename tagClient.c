#include <string.h>
#include <unistd.h>

#include "snet.h"
#include "tagGameForClient.h"

#define PORT 10000
#define HOST_LEN 64
#define D_CHARA 'o'
#define D_SX 1
#define D_SY 1
#define PLAYER_CHARA 'x'
#define PLAYER_SX 10
#define PLAYER_SY 10
#define PLAYER_LIFE 5

int main(int argc, char *argv[])
{
  char serverName[HOST_LEN];
  int s;
  TagGame *game;

  game = initTagGameForClient(D_CHARA, D_SX, D_SY, PLAYER_CHARA, PLAYER_SX, PLAYER_SY, PLAYER_LIFE);

  if (argc == 2)
    strcpy(serverName, argv[1]);
  else
    gethostname(serverName, HOST_LEN);

  s = setupClient(serverName, PORT);

  setupTagGameForClient(game, s);
  setupMazeForClient(game);

  playTagGameForClient(game);

  destroyTagGameForClient(game);

  return 0;
}
