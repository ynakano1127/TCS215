#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "tagGame.h"

#define MAINWIN_LINES   20
#define MAINWIN_COLUMS  40
#define MAINWIN_SX      2
#define MAINWIN_SY      1

#define MOVE_UP         'i'
#define MOVE_LEFT       'j'
#define MOVE_DOWN       'k'
#define MOVE_RIGHT      'l'

#define SERVER_MSG_LEN   (8 + 8 + 1)

#define CLIENT_MSG_LEN   (4 + 1)

typedef struct {
  int myKey;
  int itKey;
  int quit;
} ServerInputData;

typedef struct {
  int myKey;
  int myX;
  int myY;
  int itX;
  int itY;
  int quit;
  int dead;
} ClientInputData;

static void getServerInputData(TagGame *game, ServerInputData *serverData);
static void getClientInputData(TagGame *game, ClientInputData *clientData);
static int updatePlayerStatus(TagGame *game, ServerInputData *serverData);
static void copyGameState(TagGame *game, ClientInputData *clientData);
static void printGame(TagGame *game);
static void sendGameInfo(TagGame *game);
static void sendMyPressedKey(TagGame *game, ClientInputData *clietData);
static void die();
static void printMessage(WINDOW *mw, char *message, int length);


TagGame* initTagGame(char myChara, int mySX, int mySY,
                     char itChara, int itSX, int itSY)
{
  TagGame* game = (TagGame *)malloc(sizeof(TagGame));

  bzero(game, sizeof(TagGame));

  game->my.chara = myChara;
  game->my.x     = mySX;
  game->my.y     = mySY;
  game->it.chara = itChara;
  game->it.x     = itSX;
  game->it.y     = itSY;

  memcpy(&game->preMy, &game->my, sizeof(Player));
  memcpy(&game->preIt, &game->it, sizeof(Player));

  initscr();
  signal(SIGINT, die);
  signal(SIGTERM, die);
  noecho();
  cbreak();

  game->mainWin = newwin(MAINWIN_LINES, MAINWIN_COLUMS, MAINWIN_SY, MAINWIN_SX);
  
  if (game->mainWin == NULL) {
    endwin();
    fprintf(stderr, "Error: terminal size is too small\n");
    exit(1);
  }

  keypad(game->mainWin, TRUE);

  return game;
}

void setupTagGame(TagGame *game, int s)
{
  game->s = s;
  FD_ZERO(&(game->fdset));
  FD_SET(0, &(game->fdset));
  FD_SET(s, &(game->fdset));
  game->fdsetWidth = s + 1;
  game->watchTime.tv_sec  = 0;
  game->watchTime.tv_usec = 100 * 1000;

  box(game->mainWin, ACS_VLINE, ACS_HLINE);

  wrefresh(game->mainWin);
}

void playServerTagGame(TagGame *game)
{
  ServerInputData serverData;

  while (1) {
    getServerInputData(game, &serverData);

    if (serverData.quit)
      break;

    int win = updatePlayerStatus(game, &serverData);

    if(win){
      write(game->s, "dead", 5);  
      printMessage(game->mainWin, "WIN", 3);
      sleep(3);
      break;
    }

    printGame(game);

    sendGameInfo(game);
  }

  write(game->s, "quit", 5);  
}

void playClientTagGame(TagGame *game)
{
  ClientInputData clientData;

  while (1) {
    getClientInputData(game, &clientData);

    if (clientData.quit)
      break;

    if(clientData.dead){
      printMessage(game->mainWin, "LOSE", 4);
      sleep(3);
      break;
    }

    copyGameState(game, &clientData);

    printGame(game);

    sendMyPressedKey(game, &clientData);
  }

  write(game->s, "quit", 5);  
}

void destroyTagGame(TagGame *game)
{
  delwin(game->mainWin);
  close(game->s);
  free(game);
  die();
}


static void getServerInputData(TagGame *game, ServerInputData *serverData)
{
  fd_set  arrived   = game->fdset;
  TimeVal watchTime = game->watchTime;
  char    msg[CLIENT_MSG_LEN];

  bzero(serverData, sizeof(ServerInputData));
  select(game->fdsetWidth, &arrived, NULL, NULL, &watchTime);
  
  if (FD_ISSET(0, &arrived)) {
    serverData->myKey = wgetch(game->mainWin);
    if (serverData->myKey == 'q')
      serverData->quit = TRUE;
  }

  if (FD_ISSET(game->s, &arrived)) {
    read(game->s, msg, CLIENT_MSG_LEN);
    
    if (strcmp(msg, "quit") == 0)
      serverData->quit = TRUE;
    else 
      sscanf(msg, "%3d ", &serverData->itKey);
  }

  usleep(watchTime.tv_usec); 
  if (serverData->myKey != 0)  
    flushinp(); 
}

static void getClientInputData(TagGame *game, ClientInputData *clientData)
{
  fd_set  arrived   = game->fdset;
  TimeVal watchTime = game->watchTime;
  char    msg[SERVER_MSG_LEN];

  bzero(clientData, sizeof(ClientInputData));

  select(game->fdsetWidth, &arrived, NULL, NULL, &watchTime);

  if (FD_ISSET(0, &arrived)) {
    clientData->myKey = wgetch(game->mainWin);
    if (clientData->myKey == 'q')
      clientData->quit = TRUE;
  }

  if (FD_ISSET(game->s, &arrived)) {
    read(game->s, msg, SERVER_MSG_LEN);
    lseek(game->s, 0, SEEK_END);

    if (strcmp(msg, "quit") == 0)
      clientData->quit = TRUE;
    if (strcmp(msg, "dead") == 0)
      clientData->dead = TRUE;
    else {
      sscanf(msg, "%3d %3d %3d %3d ", &clientData->itX, &clientData->itY, 
                                      &clientData->myX, &clientData->myY);
    }
  }

  usleep(watchTime.tv_usec); 
  if (clientData->myKey != 0)  
    flushinp(); 
}

static int updatePlayerStatus(TagGame *game, ServerInputData *serverData)
{
  Player *my = &game->my;
  Player *it = &game->it;

  memcpy(&game->preMy, &game->my, sizeof(Player));
  memcpy(&game->preIt, &game->it, sizeof(Player));
  
  switch (serverData->myKey) {
  case KEY_UP: 
  case MOVE_UP: 
    if (my->y > 1) my->y--;
    break;

  case KEY_DOWN: 
  case MOVE_DOWN:
    if (my->y < MAINWIN_LINES - 2) my->y++;
    break;

  case KEY_LEFT: 
  case MOVE_LEFT:
    if (my->x > 1) my->x--;
    break;

  case KEY_RIGHT: 
  case MOVE_RIGHT:
    if (my->x < MAINWIN_COLUMS - 2) my->x++;
    break;
  }

  switch (serverData->itKey) {
  case KEY_UP: 
  case MOVE_UP: 
    if (it->y > 1) it->y--;
    break;

  case KEY_DOWN: 
  case MOVE_DOWN:
    if (it->y < MAINWIN_LINES - 2) it->y++;
    break;

  case KEY_LEFT: 
  case MOVE_LEFT:
    if (it->x > 1) it->x--;
    break;

  case KEY_RIGHT: 
  case MOVE_RIGHT:
    if (it->x < MAINWIN_COLUMS - 2) it->x++;
    break;
  }

  if(my->x == it->x && my->y == it->y){
    return TRUE;
  }

  return FALSE;
}

static void copyGameState(TagGame *game, ClientInputData *clientData)
{
  Player *my = &game->my;
  Player *it = &game->it;

  if (clientData->myX == 0) 
    return; 

  memcpy(&game->preMy, &game->my, sizeof(Player));
  memcpy(&game->preIt, &game->it, sizeof(Player));
  
  my->x = clientData->myX;
  my->y = clientData->myY;
  it->x = clientData->itX;
  it->y = clientData->itY;
}

static void printGame(TagGame *game)
{
  WINDOW *mw    = game->mainWin;
  Player *my    = &game->my;    
  Player *preMy = &game->preMy; 
  Player *it    = &game->it;    
  Player *preIt = &game->preIt; 

  mvwaddch(mw, preIt->y, preIt->x, ' ');
  mvwaddch(mw, it->y, it->x, it->chara);

  mvwaddch(mw, preMy->y, preMy->x, ' ');
  mvwaddch(mw, my->y, my->x, my->chara);

  wrefresh(mw);
}

static void printMessage(WINDOW *mw, char *message, int length)
{
  int beg_y, beg_x, size_x, size_y;
  getbegyx(mw, beg_y, beg_x);
  getmaxyx(mw, size_y, size_x);

  int center_y = size_y/2, center_x = size_x/2;

  WINDOW *message_win;
  message_win = newwin(3, length+2, beg_y+center_y-1, beg_x+center_x-length/2);

  box(message_win, '|', '-');
  wmove(message_win, 1, 1);
  wprintw(message_win,"%s", message);

  wrefresh(message_win);
}

static void sendGameInfo(TagGame *game)
{
  Player *my = &game->my;    
  Player *it = &game->it;    
  char    msg[SERVER_MSG_LEN];

  if (memcmp(&game->my, &game->preMy, sizeof(Player)) == 0 &&
      memcmp(&game->it, &game->preIt, sizeof(Player)) == 0)
    return;

  sprintf(msg, "%3d %3d %3d %3d", my->x, my->y, it->x, it->y);

  write(game->s, msg, SERVER_MSG_LEN);    
}

static void sendMyPressedKey(TagGame *game, ClientInputData *clietData)
{
  char    msg[CLIENT_MSG_LEN];

  if (clietData->myKey == 0)
    return;

  sprintf(msg, "%d", clietData->myKey);

  write(game->s, msg, CLIENT_MSG_LEN);    
}

static void die()
{
  endwin();
  exit(1);
}
