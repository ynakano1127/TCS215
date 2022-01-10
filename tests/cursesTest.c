#include <curses.h>
#include <unistd.h>

#define LINES_WIN 8   // 論理ウィンドウの高さ(行数)
#define COLUMS_WIN 40 // 論理ウィンドウの横幅(桁数)
#define SX_WIN 4      // 論理ウィンドウの左上座標
#define SY_WIN 2      // 論理ウィンドウの左上座標

static void printMessage(WINDOW *mw, char *message, int length);

int main(int argc, char *argv[])
{
  WINDOW *win;

  initscr();
  win = newwin(LINES_WIN, COLUMS_WIN, SY_WIN, SX_WIN);
  box(win, '|', '-');
  wrefresh(win);

  printMessage(win, "hogehoge", 8);

  sleep(1);

  delwin(win);
  endwin();

  return 0;
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
