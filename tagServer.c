#include "snet.h"
#include "tagGameForServer.h"

#define PORT 10000
#define D_CHARA 'o'
#define D_SX 1
#define D_SY 1
#define PLAYER_CHARA 'x'
#define PLAYER_SX 10
#define PLAYER_SY 10
#define PLAYER_LIFE 5

int main(int argc, char *argv[])
{
  int s;
  TagGame *game;

  game = initTagGameForServer(D_CHARA, D_SX, D_SY, PLAYER_CHARA, PLAYER_SX, PLAYER_SY, PLAYER_LIFE);

  s = setupServer(PORT);

  setupTagGameForServer(game, s);
  setupMazeForServer(game);

  playTagGameForServer(game);

  sleep(1);

  destroyTagGameForServer(game);

  return 0;
}
