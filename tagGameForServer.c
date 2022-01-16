#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "tagGameForServer.h"
#include "maze.h"

static void getServerInputData(TagGameForServer *tgamefs, ServerInputData *serverData);
static int updatePlayerStatus(TagGameForServer *tgamefs, ServerInputData *serverData);
static void updateBulletsStatus(TagGame *game);
static void sendGameInfo(TagGameForServer *tgamefs);
static int canGoThrough(TagGame *game, int y, int x);
static void sendMessage(TagGameForServer *tgamefs, char *msg);

TagGameForServer *initTagGameForServer(char dChara, int dSX, int dSY)
{
  TagGameForServer *tgamefs = (TagGameForServer *)malloc(sizeof(TagGameForServer));

  for (int i = 0; i < MAX_CLIENT_NUM; i++)
    tgamefs->isAlived[i] = FALSE;

  TagGame *game = &tgamefs->game;
  bzero(game, sizeof(TagGame));

  game->demon.chara = dChara;
  game->demon.x = dSX;
  game->demon.y = dSY;
  game->playerNumber = MAX_CLIENT_NUM;

  game->bullet_num = 0;
  game->bullets = (Bullet *)malloc(sizeof(Bullet) * BULLET_NUM_MEX);

  initscr();

  start_color();
  assume_default_colors(COLOR_WHITE, COLOR_BLACK);
  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  curs_set(0);

  signal(SIGINT, die);
  signal(SIGTERM, die);
  noecho();
  cbreak();

  return tgamefs;
}

void setupTagGameForServer(TagGameForServer *tgamefs, char pChara, int pSX, int pSY, int pLife)
{
  TagGame *game = &tgamefs->game;
  Clients *clients = &tgamefs->clients;

  game->player_num = clients->num;
  for (int i = 0; i < clients->num; i++)
  {
    tgamefs->isAlived[i] = TRUE;
    game->players[i].chara = pChara;
    game->players[i].x = pSX;
    game->players[i].y = pSY;
    game->players[i].life = pLife;
  }

  FD_ZERO(&(game->fdset));
  FD_SET(0, &(game->fdset));
  for (int i = 0; i < clients->num; i++)
    FD_SET(clients->fd[i], &(game->fdset));
  game->fdsetWidth = clients->fd[clients->num - 1] + 1;
  game->watchTime.tv_sec = 0;
  game->watchTime.tv_usec = 100 * 1000;
}

void setupMazeForServer(TagGameForServer *tgamefs)
{
  fprintf(stderr, "hello");
  TagGame *game = &tgamefs->game;

  game->mazeHeight = MAIN_WIN_HEIGHT;
  game->mazeWidth = MAIN_WIN_WIDTH;
  game->maze = makeMaze(game->mazeHeight, game->mazeWidth);

  char msg[MSG_LEN];
  sprintf(msg, "%3d %3d", game->mazeHeight, game->mazeWidth);
  sendMessage(tgamefs, msg);
  fprintf(stderr, "hello2");

  for (int h = 0; h < game->mazeHeight; h++)
  {
    for (int w = 0; w < game->mazeWidth; w++)
    {
      sprintf(msg, "%3d", game->maze[h][w]);
      sendMessage(tgamefs, msg);
    }
  }
  fprintf(stderr, "hello3");

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

void playTagGameForServer(TagGameForServer *tgamefs)
{
  TagGame *game = &tgamefs->game;
  // Clients *clients = &tgamefs->clients;
  // int *isAlived = tgamefs->isAlived;

  ServerInputData serverData;

  while (1)
  {
    getServerInputData(tgamefs, &serverData);

    if (serverData.quit)
      break;

    if (serverData.bullet != 0 && game->bullet_num <= BULLET_NUM_MEX)
    {
      game->bullets[game->bullet_num].x = game->demon.x;
      game->bullets[game->bullet_num].y = game->demon.y;
      game->bullets[game->bullet_num].speed_type = serverData.bullet;
      game->bullet_num++;
    }
    updateBulletsStatus(game);

    if (updatePlayerStatus(tgamefs, &serverData))
    {
      sendMessage(tgamefs, "dead");
      printMessage(game->mainWin, "WIN", 3);
      sleep(3);
      break;
    }

    printGame(game);

    sendGameInfo(tgamefs);
  }

  sendMessage(tgamefs, "quit");
}

void destroyTagGameForServer(TagGameForServer *tgamefs)
{
  delwin(tgamefs->game.mainWin);
  for (int i = 0; i < tgamefs->clients.num; i++)
    close(tgamefs->clients.fd[i]);
  free(tgamefs);
  die();
}

static void getServerInputData(TagGameForServer *tgamefs, ServerInputData *serverData)
{
  TagGame *game = &tgamefs->game;
  Clients *clients = &tgamefs->clients;
  int *isAlived = tgamefs->isAlived;

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

    switch (serverData->myKey)
    {
    case 'w':
      serverData->bullet = BULLET_SPPED_TYPE_UP;
      break;
    case 'z':
      serverData->bullet = BULLET_SPPED_TYPE_DOWN;
      break;
    case 'd':
      serverData->bullet = BULLET_SPPED_TYPE_RIGHT;
      break;
    case 'a':
      serverData->bullet = BULLET_SPPED_TYPE_LEFT;
      break;

    default:
      break;
    }
  }

  for (int i = 0; i < clients->num; i++)
  {
    if (!isAlived[i])
      continue;
    if (!FD_ISSET(clients->fd[i], &arrived))
      continue;

    if (FD_ISSET(clients->fd[i], &arrived))
    {
      read(clients->fd[i], msg, CLIENT_MSG_LEN);

      if (strcmp(msg, "quit") == 0)
        serverData->quit = TRUE;
      else
        sscanf(msg, "%3d ", &serverData->itKeys[i]);
    }
  }

  usleep(watchTime.tv_usec);
  if (serverData->myKey != 0)
    flushinp();
}
static void updateBulletsStatus(TagGame *game)
{
  for (int i = 0; i < game->bullet_num; i++)
  {
    Bullet b = game->bullets[i];
    if (!canGoThrough(game, b.y, b.x))
      continue;

    switch (b.speed_type)
    {
    case BULLET_SPPED_TYPE_UP:
      game->bullets[i].y--;
      break;
    case BULLET_SPPED_TYPE_DOWN:
      game->bullets[i].y++;
      break;
    case BULLET_SPPED_TYPE_LEFT:
      game->bullets[i].x--;
      break;
    case BULLET_SPPED_TYPE_RIGHT:
      game->bullets[i].x++;
      break;

    default:
      break;
    }
  }
}

static int updatePlayerStatus(TagGameForServer *tgamefs, ServerInputData *serverData)
{
  TagGame *game = &tgamefs->game;
  Clients *clients = &tgamefs->clients;

  Demon *my = &game->demon;
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

  for (int i = 0; i < clients->num; i++)
  {
    if (!serverData->itKeys[i])
      continue;
    Player *it = &game->players[i];
    switch (serverData->itKeys[i])
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
  }

  for (int i = 0; i < clients->num; i++)
  {
    Player *it = &game->players[i];
    if (my->x == it->x && my->y == it->y)
    {
      return TRUE;
    }
  }

  for (int i = 0; i < game->bullet_num; i++)
  {
    Bullet b = game->bullets[i];
    for (int i = 0; i < clients->num; i++)
    {
      Player *it = &game->players[i];
      if (b.x == it->x && b.y == it->y)
      {
        it->life--;
        if (it->life < 0)
          return TRUE;
      }
    }
  }

  return FALSE;
}

static void sendGameInfo(TagGameForServer *tgamefs)
{
  TagGame *game = &tgamefs->game;
  Clients *clients = &tgamefs->clients;
  int *isAlived = tgamefs->isAlived;

  Demon *my = &game->demon;
  char msg[SERVER_MSG_LEN];

  for (int i = 0; i < clients->num; i++)
  {
    sprintf(msg, "%3d %3d %3d %3d", i, my->x, my->y, clients->num);
    if (isAlived[i])
      write(clients->fd[i], msg, SERVER_MSG_LEN);
  }

  for (int i = 0; i < clients->num; i++)
  {
    Player *it = &game->players[i];
    sprintf(msg, "%3d %3d %3d", it->x, it->y, it->life);
    sendMessage(tgamefs, msg);
  }

  char msg2[SERVER_MSG_LEN];
  sprintf(msg2, "%3d", game->bullet_num);
  sendMessage(tgamefs, msg2);

  char msg3[SERVER_MSG_LEN];
  for (int i = 0; i < game->bullet_num; i++)
  {
    Bullet b = game->bullets[i];
    sprintf(msg3, "%3d %3d %3d", b.x, b.y, b.speed_type);
    sendMessage(tgamefs, msg3);
  }
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

static void sendMessage(TagGameForServer *tgamefs, char *msg)
{
  Clients *clients = &tgamefs->clients;
  int *isAlived = tgamefs->isAlived;

  for (int i = 0; i < clients->num; i++)
    if (isAlived[i])
      write(clients->fd[i], msg, SERVER_MSG_LEN);
}
