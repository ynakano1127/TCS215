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

  // bzero(game, sizeof(TagGame));

  game->demon.chara = dChara;
  game->demon.x = dSX;
  game->demon.y = dSY;
  for (int i = 0; i < MAX_CLIENT_NUM; i++)
  {
    game->players[i].chara = pChara;
    game->players[i].x = pSX;
    game->players[i].y = pSY;
    game->players[i].life = pLife;
  }
  game->playerNumber = 0;

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

  game->lifeWin = newwin(LIFE_WIN_HEIGHT, LIFE_WIN_WIDTH, LIFEWIN_SY, LIFEWIN_SX);

  if (game->lifeWin == NULL)
  {
    endwin();
    fprintf(stderr, "Error: terminal size is too small\n");
    exit(1);
  }

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
}

void setupMazeForClient(TagGame *game)
{
  char msg[SERVER_MSG_LEN];
  read(game->s, msg, SERVER_MSG_LEN);
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
      read(game->s, msg, SERVER_MSG_LEN);
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

  // wrefresh(game->mainWin);
}

void playTagGameForClient(TagGame *game)
{
  ClientInputData clientData;
  Bullet *bs_address = (Bullet *)malloc(sizeof(Bullet) * BULLET_NUM_MEX);

  while (1)
  {
    bzero(&clientData, sizeof(ClientInputData));
    clientData.bullet = bs_address;
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
      sscanf(msg, "%3d %3d %3d %3d", &clientData->player_my_number, &clientData->itX, &clientData->itY, &clientData->player_num);

      for (int i = 0; i < clientData->player_num; i++)
      {
        read(game->s, msg, SERVER_MSG_LEN);
        lseek(game->s, 0, SEEK_END);
        sscanf(msg, "%3d %3d %3d", &clientData->players[i].x, &clientData->players[i].y, &clientData->players[i].life);
      }

      read(game->s, msg, SERVER_MSG_LEN);
      lseek(game->s, 0, SEEK_END);
      sscanf(msg, "%3d", &clientData->bullet_num);

      for (int i = 0; i < clientData->bullet_num; i++)
      {
        read(game->s, msg, SERVER_MSG_LEN);
        lseek(game->s, 0, SEEK_END);
        sscanf(msg, "%3d %3d %3d", &clientData->bullet[i].x, &clientData->bullet[i].y, &clientData->bullet[i].speed_type);
      }

      // printf("player_my_number = %d\n", clientData->player_my_number);
      // printf("itX = %d\n", clientData->itX);
      // printf("itY = %d\n", clientData->itY);
      // printf("playersx = %d\n", clientData->players[clientData->player_my_number].x);
      // printf("playersy = %d\n", clientData->players[clientData->player_my_number].y);
      // printf("player_num = %d\n", clientData->player_num);
      // printf("bullet_num = %d\n", clientData->bullet_num);
    }
  }

  usleep(watchTime.tv_usec);
  if (clientData->myKey != 0)
    flushinp();
}

static void copyGameState(TagGame *game, ClientInputData *clientData)
{
  if (clientData->player_num == 0)
    return;

  game->player_num = clientData->player_num;
  game->playerNumber = clientData->player_my_number;
  for(int i=0;i<clientData->player_num;i++){
    game->players[i].x = clientData->players[i].x;
    game->players[i].y = clientData->players[i].y;
    game->players[i].life = clientData->players[i].life;
  }  

  game->demon.x = clientData->itX;
  game->demon.y = clientData->itY;

  game->bullet_num = clientData->bullet_num;
  for (int i = 0; i < clientData->bullet_num; i++)
  {
    game->bullets[i].x = clientData->bullet[i].x;
    game->bullets[i].y = clientData->bullet[i].y;
    game->bullets[i].speed_type = clientData->bullet[i].speed_type;
  }
  // printf("player_my_number = %d\n", clientData->player_my_number);
  // printf("itX = %d\n", clientData->itX);
  // printf("itY = %d\n", clientData->itY);
  // printf("playersx = %d\n", clientData->players[clientData->player_my_number].x);
  // printf("playersy = %d\n", clientData->players[clientData->player_my_number].y);
  // printf("player_num = %d\n", clientData->player_num);
  // printf("bullet_num = %d\n", clientData->bullet_num);
  // printf("player_num = %d\n", game->player_num);
  // printf("playerNumber = %d\n", game->playerNumber);
  // printf("game->players[i].x = %d\n", game->players[game->playerNumber].x);
  // printf("game->players[i].x = %d\n", game->players[game->playerNumber].x);
  // printf("game->daemon.x = %d\n", game->demon.x);
  // printf("game->daemon.y = %d\n", game->demon.y);
}

static void sendMyPressedKey(TagGame *game, ClientInputData *clietData)
{
  char msg[CLIENT_MSG_LEN];

  if (clietData->myKey == 0)
    return;

  sprintf(msg, "%d", clietData->myKey);

  write(game->s, msg, CLIENT_MSG_LEN);
}
