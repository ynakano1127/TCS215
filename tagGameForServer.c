#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "tagGameForServer.h"
#include "maze.h"

static void getServerInputData(TagGame *game, ServerInputData *serverData);
static int updatePlayerStatus(TagGame *game, ServerInputData *serverData);
static void sendGameInfo(TagGame *game);
static int canGoThrough(TagGame *game, int y, int x);

TagGame *initTagGameForServer(char myChara, int mySX, int mySY, int myLife,
                              char itChara, int itSX, int itSY, int itLife)
{
  TagGame *game = (TagGame *)malloc(sizeof(TagGame));

  bzero(game, sizeof(TagGame));

  game->my.chara = myChara;
  game->my.x = mySX;
  game->my.y = mySY;
  game->my.life = myLife;
  game->it.chara = itChara;
  game->it.x = itSX;
  game->it.y = itSY;
  game->it.life = itLife;

  memcpy(&game->preMy, &game->my, sizeof(Player));
  memcpy(&game->preIt, &game->it, sizeof(Player));

  initscr();

  start_color();
  assume_default_colors(COLOR_WHITE, COLOR_BLACK);
  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  curs_set(0);

  signal(SIGINT, die);
  signal(SIGTERM, die);
  noecho();
  cbreak();

  return game;
}

void setupTagGameForServer(TagGame *game, int s)
{
  game->s = s;
  FD_ZERO(&(game->fdset));
  FD_SET(0, &(game->fdset));
  FD_SET(s, &(game->fdset));
  game->fdsetWidth = s + 1;
  game->watchTime.tv_sec = 0;
  game->watchTime.tv_usec = 100 * 1000;

  game->lifeWin = newwin(LIFE_WIN_HEIGHT, LIFE_WIN_WIDTH, LIFEWIN_SY, LIFEWIN_SX);

  if (game->lifeWin == NULL)
  {
    endwin();
    fprintf(stderr, "Error: terminal size is too small\n");
    exit(1);
  }
}

void setupMazeForServer(TagGame *game)
{
  game->mazeHeight = MAIN_WIN_HEIGHT;
  game->mazeWidth = MAIN_WIN_WIDTH;
  game->maze = makeMaze(game->mazeHeight, game->mazeWidth);

  char msg[MSG_LEN];
  sprintf(msg, "%3d %3d", game->mazeHeight, game->mazeWidth);
  write(game->s, msg, MSG_LEN);

  for (int h = 0; h < game->mazeHeight; h++)
  {
    for (int w = 0; w < game->mazeWidth; w++)
    {
      sprintf(msg, "%3d", game->maze[h][w]);
      write(game->s, msg, MSG_LEN);
    }
  }

  game->mainWin = newwin(game->mazeHeight, game->mazeWidth, MAINWIN_SY, MAINWIN_SX);

  if (game->mainWin == NULL)
  {
    endwin();
    fprintf(stderr, "Error: terminal size is too small\n");
    exit(1);
  }

  keypad(game->mainWin, TRUE);
  box(game->mainWin, ACS_VLINE, ACS_HLINE);

  wrefresh(game->mainWin);
}

void playTagGameForServer(TagGame *game)
{
  ServerInputData serverData;

  while (1)
  {
    getServerInputData(game, &serverData);

    if (serverData.quit)
      break;

    int win = updatePlayerStatus(game, &serverData);

    if (win)
    {
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

void destroyTagGameForServer(TagGame *game)
{
  delwin(game->mainWin);
  close(game->s);
  free(game);
  die();
}

static void getServerInputData(TagGame *game, ServerInputData *serverData)
{
  fd_set arrived = game->fdset;
  TimeVal watchTime = game->watchTime;
  char msg[CLIENT_MSG_LEN];

  bzero(serverData, sizeof(ServerInputData));
  select(game->fdsetWidth, &arrived, NULL, NULL, &watchTime);

  if (FD_ISSET(0, &arrived))
  {
    serverData->myKey = wgetch(game->mainWin);
    if (serverData->myKey == 'q')
      serverData->quit = TRUE;
  }

  if (FD_ISSET(game->s, &arrived))
  {
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

static int updatePlayerStatus(TagGame *game, ServerInputData *serverData)
{
  Player *my = &game->my;
  Player *it = &game->it;

  memcpy(&game->preMy, &game->my, sizeof(Player));
  memcpy(&game->preIt, &game->it, sizeof(Player));

  switch (serverData->myKey)
  {
  case KEY_UP:
  case MOVE_UP:
    if (canGoThrough(game, my->y - 1, my->x))
      my->y--;
    break;

  case KEY_DOWN:
  case MOVE_DOWN:
    if (canGoThrough(game, my->y + 1, my->x))
      my->y++;
    break;

  case KEY_LEFT:
  case MOVE_LEFT:
    if (canGoThrough(game, my->y, my->x - 1))
      my->x--;
    break;

  case KEY_RIGHT:
  case MOVE_RIGHT:
    if (canGoThrough(game, my->y, my->x + 1))
      my->x++;
    break;
  }

  switch (serverData->itKey)
  {
  case KEY_UP:
  case MOVE_UP:
    if (canGoThrough(game, it->y - 1, it->x))
      it->y--;
    break;

  case KEY_DOWN:
  case MOVE_DOWN:
    if (canGoThrough(game, it->y + 1, it->x))
      it->y++;
    break;

  case KEY_LEFT:
  case MOVE_LEFT:
    if (canGoThrough(game, it->y, it->x - 1))
      it->x--;
    break;

  case KEY_RIGHT:
  case MOVE_RIGHT:
    if (canGoThrough(game, it->y, it->x + 1))
      it->x++;
    break;
  }

  if (my->x == it->x && my->y == it->y)
  {
    return TRUE;
  }

  return FALSE;
}

static void sendGameInfo(TagGame *game)
{
  Player *my = &game->my;
  Player *it = &game->it;
  char msg[SERVER_MSG_LEN];

  if (memcmp(&game->my, &game->preMy, sizeof(Player)) == 0 &&
      memcmp(&game->it, &game->preIt, sizeof(Player)) == 0)
    return;

  sprintf(msg, "%3d %3d %3d %3d", my->x, my->y, it->x, it->y);

  write(game->s, msg, SERVER_MSG_LEN);
}

static int canGoThrough(TagGame *game, int y, int x)
{
  if (y < 0 || game->mazeHeight <= y || x < 0 || game->mazeWidth <= x)
    return 0;

  if (game->maze[y][x] == 1)
  {
    return 0;
  }
  return 1;
}
