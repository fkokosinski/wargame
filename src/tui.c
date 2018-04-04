#include "tui.h"

void tui_init(void) {
    initscr();
    curs_set(0);
}

/* print game title */
void print_title(void) {
    attron(A_BOLD);
    mvprintw(0, COLS/2 - (int)strlen(GAME_TITLE)/2, GAME_TITLE);
    attroff(A_BOLD);
    refresh();
}

void tui_resize(int sig) {
    signal(SIGWINCH, SIG_IGN);

    endwin();
    initscr();

    signal(SIGWINCH, tui_resize);
}
