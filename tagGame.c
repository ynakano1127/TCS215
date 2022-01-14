#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "tagGame.h"
#include "maze.h"

void printGame(TagGame *game)
{
  WINDOW *mw = game->mainWin;
  Player *my = &game->my;
  Player *it = &game->it;

  for (int h = 0; h < game->mazeHeight; h++)
  {
    for (int w = 0; w < game->mazeWidth; w++)
    {
      if (game->maze[h][w] == 1)
      {
        wattron(game->mainWin, COLOR_PAIR(1));
        mvwaddch(mw, h, w, ' ');
        wattroff(game->mainWin, COLOR_PAIR(1));
      }
      else
      {
        mvwaddch(mw, h, w, ' ');
      }
    }
  }

  mvwaddch(mw, it->y, it->x, it->chara);
  mvwaddch(mw, my->y, my->x, my->chara);

  WINDOW *lw = game->lifeWin;
  box(lw, '|', '-');
  wmove(lw, 2, 1);
  wprintw(lw, "LIFE: %d", game->my.life);

  wrefresh(mw);
  wrefresh(lw);
}

void printMessage(WINDOW *mw, char *message, int length)
{
  int beg_y, beg_x, size_x, size_y;
  getbegyx(mw, beg_y, beg_x);
  getmaxyx(mw, size_y, size_x);

  int center_y = size_y / 2, center_x = size_x / 2;

  WINDOW *message_win;
  message_win = newwin(3, length + 2, beg_y + center_y - 1, beg_x + center_x - length / 2);

  box(message_win, '|', '-');
  wmove(message_win, 1, 1);
  wprintw(message_win, "%s", message);

  wrefresh(message_win);
}

void die()
{
  endwin();
  exit(1);
}
