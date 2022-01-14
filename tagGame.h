#include <curses.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define MAINWIN_SX 2
#define MAINWIN_SY 1

#define MAIN_WIN_WIDTH 41
#define MAIN_WIN_HEIGHT 21

#define LIFEWIN_SX 50
#define LIFEWIN_SY 1

#define LIFE_WIN_WIDTH 10
#define LIFE_WIN_HEIGHT 5

#define MSG_LEN 20
#define SERVER_MSG_LEN (8 + 8 + 1)
#define CLIENT_MSG_LEN (4 + 1)

#define MOVE_UP 'i'
#define MOVE_LEFT 'j'
#define MOVE_DOWN 'k'
#define MOVE_RIGHT 'l'

typedef struct timeval TimeVal;

typedef struct
{
  char chara;
  int x;
  int y;
  int life;
} Player;

typedef struct
{
  Player my;
  Player it;

  WINDOW *mainWin;
  WINDOW *lifeWin;

  int mazeHeight;
  int mazeWidth;
  int **maze;

  int s;
  fd_set fdset;
  int fdsetWidth;
  TimeVal watchTime;
} TagGame;

typedef struct
{
  int myKey;
  int itKey;
  int quit;
} ServerInputData;

typedef struct
{
  int myKey;
  int myX;
  int myY;
  int itX;
  int itY;
  int quit;
  int dead;
} ClientInputData;

void printGame(TagGame *game);
void die();
void printMessage(WINDOW *mw, char *message, int length);
