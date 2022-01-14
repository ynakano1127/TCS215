#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "tagGameForClient.h"
#include "maze.h"

static void getClientInputData(TagGame *game, ClientInputData *clientData);
static void copyGameState(TagGame *game, ClientInputData *clientData);
static void sendMyPressedKey(TagGame *game, ClientInputData *clietData);

TagGame *initTagGameForClient(char dChara, int dSX, int dSY,
                              char pChara, int pSX, int pSY, int pLife)
{
  TagGame *game = (TagGame *)malloc(sizeof(TagGame));

  bzero(game, sizeof(TagGame));

  game->demon.chara = dChara;
  game->demon.x = dSX;
  game->demon.y = dSY;
  game->player.chara = pChara;
  game->player.x = pSX;
  game->player.y = pSY;
  game->player.life = pLife;

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

void setupTagGameForClient(TagGame *game, int s)
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

void setupMazeForClient(TagGame *game)
{
  char msg[MSG_LEN];
  read(game->s, msg, MSG_LEN);
  sscanf(msg, "%3d %3d", &game->mazeHeight, &game->mazeWidth);
  fprintf(stderr, "%d %d\n", game->mazeHeight, game->mazeWidth);

  game->maze = (int **)malloc(sizeof(int *) * game->mazeHeight);
  for (int i = 0; i < game->mazeHeight; i++)
  {
    game->maze[i] = (int *)malloc(sizeof(int) * game->mazeWidth);
  }

  for (int h = 0; h < game->mazeHeight; h++)
  {
    for (int w = 0; w < game->mazeWidth; w++)
    {
      read(game->s, msg, MSG_LEN);
      sscanf(msg, "%3d", &game->maze[h][w]);
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

void playTagGameForClient(TagGame *game)
{
  ClientInputData clientData;

  while (1)
  {
    getClientInputData(game, &clientData);

    if (clientData.quit)
      break;

    if (clientData.dead)
    {
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

void destroyTagGameForClient(TagGame *game)
{
  delwin(game->mainWin);
  close(game->s);
  free(game);
  die();
}

static void getClientInputData(TagGame *game, ClientInputData *clientData)
{
  fd_set arrived = game->fdset;
  TimeVal watchTime = game->watchTime;
  char msg[SERVER_MSG_LEN];

  bzero(clientData, sizeof(ClientInputData));

  select(game->fdsetWidth, &arrived, NULL, NULL, &watchTime);

  if (FD_ISSET(0, &arrived))
  {
    clientData->myKey = wgetch(game->mainWin);
    if (clientData->myKey == 'q')
      clientData->quit = TRUE;
  }

  if (FD_ISSET(game->s, &arrived))
  {
    read(game->s, msg, SERVER_MSG_LEN);
    lseek(game->s, 0, SEEK_END);

    if (strcmp(msg, "quit") == 0)
      clientData->quit = TRUE;
    if (strcmp(msg, "dead") == 0)
      clientData->dead = TRUE;
    else
    {
      sscanf(msg, "%3d %3d %3d %3d ", &clientData->itX, &clientData->itY,
             &clientData->myX, &clientData->myY);
    }
  }

  usleep(watchTime.tv_usec);
  if (clientData->myKey != 0)
    flushinp();
}

static void copyGameState(TagGame *game, ClientInputData *clientData)
{
  Player *my = &game->player;
  Demon *it = &game->demon;

  if (clientData->myX == 0)
    return;

  my->x = clientData->myX;
  my->y = clientData->myY;
  it->x = clientData->itX;
  it->y = clientData->itY;
}

static void sendMyPressedKey(TagGame *game, ClientInputData *clietData)
{
  char msg[CLIENT_MSG_LEN];

  if (clietData->myKey == 0)
    return;

  sprintf(msg, "%d", clietData->myKey);

  write(game->s, msg, CLIENT_MSG_LEN);
}
