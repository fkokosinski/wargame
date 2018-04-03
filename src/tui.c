#include "tui.h"

/* print game title */
void print_title(void) {
    attron(A_BOLD);
    mvprintw(0, COLS/2 - (int)strlen(GAME_TITLE)/2, GAME_TITLE);
    attroff(A_BOLD);
    refresh();
}

void change_size(int sig) {
    signal(SIGWINCH, SIG_IGN);
    endwin();
    initscr();
    refresh();
    clear();

    print_title();
    signal(SIGWINCH, change_size);
}
