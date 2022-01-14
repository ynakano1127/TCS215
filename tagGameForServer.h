#include <curses.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "tagGame.h"

TagGame *initTagGameForServer(char dChara, int dSX, int dSY,
                              char pChara, int pSX, int pSY, int pLife);
void setupTagGameForServer(TagGame *game, int s);
void setupMazeForServer(TagGame *game);
void playTagGameForServer(TagGame *game);
void destroyTagGameForServer(TagGame *game);
