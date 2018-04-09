#include "tui.h"

static WINDOW *_win_playerinfo;
static struct player _last_info;

/* initialize or exit ncurses */
void tui_ctl(int opt) {
    if (opt == 0) {
        initscr();
        curs_set(0);

        _win_playerinfo = newwin(LINES-2, COLS/2, 2, 0);
    } else if (opt == 1) {
        endwin();
    }
}

/* print game title */
void print_title(void) {
    attron(A_BOLD);
    mvprintw(0, COLS/2 - (int)strlen(GAME_TITLE)/2, GAME_TITLE);
    attroff(A_BOLD);
    refresh();
}

/* print player info */
void print_playerinfo(struct player player_info) {
    _last_info = player_info;

    /* clear player info window */
    wclear(_win_playerinfo);

    wattron(_win_playerinfo, A_BOLD);    /* set bold */
    if (_last_info.wins != 1)          /* check what form to use (plural vs singular) */
        wprintw(_win_playerinfo, "Player %d:\t%d wins\n", _last_info.player_id, _last_info.wins);
    else
        wprintw(_win_playerinfo, "Player %d:\t%d win\n", _last_info.player_id, _last_info.wins);
    wattroff(_win_playerinfo, A_BOLD);   /* unset bold */

    /* print player's statistics */
    wprintw(_win_playerinfo, "\nResources:\t%d\n", _last_info.resources);
    wprintw(_win_playerinfo, "Workers:\t%d\n\n", _last_info.workers);
    wprintw(_win_playerinfo, "Light inf.:\t%d\n", _last_info.light_inf);
    wprintw(_win_playerinfo, "Heavy inf.:\t%d\n", _last_info.heavy_inf);
    wprintw(_win_playerinfo, "Cavalry:\t%d\n", _last_info.cavalry);

    /* refresh window */
    wrefresh(_win_playerinfo);
}

void tui_resize(int sig) {
    signal(SIGWINCH, SIG_IGN);

    endwin();
    initscr();
    clear();
    refresh();

    print_title();
    print_playerinfo(_last_info);

    signal(SIGWINCH, tui_resize);
}
