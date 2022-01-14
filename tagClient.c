#include <string.h>
#include <unistd.h>

#include "snet.h"
#include "tagGameForClient.h"

#define PORT 10000
#define HOST_LEN 64
#define MY_CHARA 'o'
#define MY_SX 10
#define MY_SY 10
#define MY_LIFE 5
#define IT_CHARA 'x'
#define IT_SX 1
#define IT_SY 1
#define IT_LIFE 5

int main(int argc, char *argv[])
{
  char serverName[HOST_LEN];
  int s;
  TagGame *game;

  game = initTagGameForClient(MY_CHARA, MY_SX, MY_SY, MY_LIFE, IT_CHARA, IT_SX, IT_SY, IT_LIFE);

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
