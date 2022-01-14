#include "snet.h"
#include "tagGameForServer.h"

#define PORT 10000
#define MY_CHARA 'o'
#define MY_SX 1
#define MY_SY 1
#define MY_LIFE 5
#define IT_CHARA 'x'
#define IT_SX 10
#define IT_SY 10
#define IT_LIFE 5

int main(int argc, char *argv[])
{
  int s;
  TagGame *game;

  game = initTagGameForServer(MY_CHARA, MY_SX, MY_SY, MY_LIFE, IT_CHARA, IT_SX, IT_SY, IT_LIFE);

  s = setupServer(PORT);

  setupTagGameForServer(game, s);
  setupMazeForServer(game);

  playTagGameForServer(game);

  sleep(1);

  destroyTagGameForServer(game);

  return 0;
}
