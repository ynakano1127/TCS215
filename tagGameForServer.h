#include <curses.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "mnet.h"
#include "tagGame.h"

typedef struct
{
  TagGame game;
  Clients clients;
  int isAlived[MAX_CLIENT_NUM];
} TagGameForServer;

TagGameForServer *initTagGameForServer(char dChara, int dSX, int dSY);
void *initClientsForServer(TagGameForServer *tgamefs);
void *initIsAlivedForServer(TagGameForServer *tgamefs);
void setupTagGameForServer(TagGameForServer *tgamefs, char pChara, int pSX, int pSY, int pLife);
void setupMazeForServer(TagGameForServer *tgamefs);
void playTagGameForServer(TagGameForServer *tgamefs);
void destroyTagGameForServer(TagGameForServer *tgamefs);
