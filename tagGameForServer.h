#include <curses.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "tagGame.h"

TagGame *initTagGameForServer(char myChara, int mySX, int mySY, int myLife,
                              char itChara, int itSX, int itSY, int itLife);
void setupTagGameForServer(TagGame *game, int s);
void setupMazeForServer(TagGame *game);
void playTagGameForServer(TagGame *game);
void destroyTagGameForServer(TagGame *game);
