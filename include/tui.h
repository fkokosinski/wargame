#ifndef WARGAME_TUI_H
#define WARGAME_TUI_H

#include "declarations.h"

#define TUI_INIT 0
#define TUI_EXIT 1


void tui_ctl(int opt);
void tui_resize(int sig);

void print_title(void);
void print_playerinfo(struct player _last_info);

#endif
