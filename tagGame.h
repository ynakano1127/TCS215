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
#define SERVER_MSG_LEN (4 * 5 + 1)
#define CLIENT_MSG_LEN (4 + 1)

#define MOVE_UP 'i'
#define MOVE_LEFT 'j'
#define MOVE_DOWN 'k'
#define MOVE_RIGHT 'l'

#define MAX_CLIENT_NUM 10

typedef struct timeval TimeVal;

typedef struct
{
  char chara;
  int x;
  int y;
} Demon;

typedef struct
{
  char chara;
  int x;
  int y;
  int life;
} Player;

#define BULLET_NUM_MEX 100
#define BULLET_SPPED_TYPE_UP 1
#define BULLET_SPPED_TYPE_DOWN 2
#define BULLET_SPPED_TYPE_RIGHT 3
#define BULLET_SPPED_TYPE_LEFT 4

typedef struct
{
  int x;
  int y;
  int speed_type;
} Bullet;

typedef struct
{
  Demon demon;

  int player_num;
  Player players[MAX_CLIENT_NUM];
  int playerNumber;

  int bullet_num;
  Bullet *bullets;

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
  int itKeys[MAX_CLIENT_NUM];
  int bullet;
  int quit;
} ServerInputData;

typedef struct
{
  int myKey;
  int player_num;
  Player players[MAX_CLIENT_NUM];
  int player_my_number;
  int mylife;
  int itX;
  int itY;
  int bullet_num;
  Bullet *bullet;
  int quit;
  int dead;
} ClientInputData;

void printGame(TagGame *game);
void die();
void printMessage(WINDOW *mw, char *message, int length);
