#include <curses.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "tagGame.h"

TagGame *initTagGameForClient(char dChara, int dSX, int dSY,
                              char pChara, int pSX, int pSY, int pLife);
void setupTagGameForClient(TagGame *game, int s);
void setupMazeForClient(TagGame *game);
void playTagGameForClient(TagGame *game);
void destroyTagGameForClient(TagGame *game);
