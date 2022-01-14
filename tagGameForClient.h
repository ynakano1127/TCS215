#include <curses.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "tagGame.h"

TagGame *initTagGameForClient(char myChara, int mySX, int mySY, int myLife,
                              char itChara, int itSX, int itSY, int itLife);
void setupTagGameForClient(TagGame *game, int s);
void setupMazeForClient(TagGame *game);
void playTagGameForClient(TagGame *game);
void destroyTagGameForClient(TagGame *game);
