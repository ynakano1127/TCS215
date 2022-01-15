#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "tagGameForServer.h"
#include "maze.h"

static void getServerInputData(TagGame *game, ServerInputData *serverData);
static int updatePlayerStatus(TagGame *game, ServerInputData *serverData);
static void updateBulletsStatus(TagGame *game);
static void sendGameInfo(TagGame *game);
static int canGoThrough(TagGame *game, int y, int x);

TagGame *initTagGameForServer(char dChara, int dSX, int dSY,
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

    if (serverData.bullet != 0 && game->bullet_num <= BULLET_NUM_MEX)
    {
      game->bullets[game->bullet_num].x = game->demon.x;
      game->bullets[game->bullet_num].y = game->demon.y;
      game->bullets[game->bullet_num].speed_type = serverData.bullet;
      game->bullet_num++;
    }
    updateBulletsStatus(game);

    if (updatePlayerStatus(game, &serverData))
    {
      game->player.life--;
    }

    if (game->player.life < 0)
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

static int updatePlayerStatus(TagGame *game, ServerInputData *serverData)
{
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

  Player *it = &game->player;
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

  for (int i = 0; i < game->bullet_num; i++)
  {
    Bullet b = game->bullets[i];
    if (b.x == it->x && b.y == it->y)
    {
      return TRUE;
    }
  }

  return FALSE;
}

static void sendGameInfo(TagGame *game)
{
  Demon *my = &game->demon;
  Player *it = &game->player;
  char msg[SERVER_MSG_LEN];

  sprintf(msg, "%3d %3d %3d %3d %3d", my->x, my->y, it->x, it->y, it->life);

  write(game->s, msg, SERVER_MSG_LEN);

  char msg2[SERVER_MSG_LEN];
  sprintf(msg2, "%3d", game->bullet_num);
  write(game->s, msg2, SERVER_MSG_LEN);

  char msg3[SERVER_MSG_LEN];
  for (int i = 0; i < game->bullet_num; i++)
  {
    Bullet b = game->bullets[i];
    sprintf(msg3, "%3d %3d %3d", b.x, b.y, b.speed_type);
    write(game->s, msg3, SERVER_MSG_LEN);
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
