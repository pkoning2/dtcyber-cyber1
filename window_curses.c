/* ui-curses.c

*/
#include <stdio.h>
#include <string.h>
#include <curses.h>

#include "const.h"
#include "types.h"
#include "proto.h"

static u16 currentX;
static u16 currentY;
static u8  currentF;

static int maxX;
static int maxY;

static WINDOW *win;
static WINDOW *pwin;

static u8 touched[63];

static void win_show_yx( void)
{
#if 0
    mvwprintw(pwin, 0, 0, "X=%04X, Y=%04X", currentX, currentY);
    mvwprintw(pwin, 0, maxX - 8, "Font=%02X", currentF);
    wrefresh(pwin);
#endif
}

void windowInit(void)
{
    int     x,
            y;

    win = initscr();
    cbreak();
    noecho();
    nodelay(win, TRUE);
    clearok( win, TRUE);

    if ( has_colors()) {
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);
        init_pair(2, COLOR_RED, COLOR_BLACK);
    }

    getmaxyx(win, y, x);
    pwin = newwin(1, x, y - 1, 0);
    maxX = x;
    maxY = y;
    memset( &touched, 0, sizeof( touched));
}

void windowSetFont(u8 font)
{
    currentF = font;
    win_show_yx();
}

void windowSetX(u16 x)
{
    currentX = x;
    win_show_yx();
}

void windowSetY(u16 y)
{
    currentY = y;
    win_show_yx();
}

void windowQueue(char ch)
{
    if (currentX || currentX) {
        int     x = (currentX >> 3);
        int     y = ((0777 - currentY) >> 3);

        if (! touched[ y]) {
            mvwaddch( win, y, 0, ' ');
            wclrtoeol( win);
        }
//      mvwaddch(win, y, x, ch | COLOR_PAIR(2));
        mvwaddch(win, y, x, ch | COLOR_PAIR(1));
        touched[ y] = 1;
        currentX = currentY = 0;
    } else {
        waddch(win, ch | COLOR_PAIR(1));
    }
}

void windowUpdate(void)
{
    int i;
    int     keyIn;


    for ( i = 0; i < sizeof( touched)/sizeof( touched[0]); i++) {
        if ( ! touched) {
            mvwaddch( win, i, 0, ' ');
            wclrtoeol( win);
        }
    }
    memset( touched, 0, sizeof( touched));

    wrefresh(win);
    keyIn = wgetch(win);
    if (keyIn != ERR) {
        ppKeyIn = keyIn;
        wclear( win);
    }
}

void windowClose(void)
{
    endwin();
}
