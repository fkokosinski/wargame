#include "client.h"

struct player player_info;
static pthread_mutex_t player_data = PTHREAD_MUTEX_INITIALIZER;

/* receive messages from server and process them accordingly */
void *receiver(void *data) {
    time_t last_msg = time(NULL);

    pthread_mutex_lock(&player_data);
    while (player_info.active) {
        pthread_mutex_unlock(&player_data);

        /* check if any messages */
        pthread_mutex_lock(&player_data);
        if (!msg_pong(&player_info)) {
            if (time(NULL) - last_msg > PING_INTERVAL)
                player_info.active = false;
            continue;
        }
        pthread_mutex_unlock(&player_data);

        last_msg = time(NULL);

        pthread_mutex_lock(&player_data);
    }
    pthread_mutex_unlock(&player_data);

    return NULL;
}

int client_main(void) {
    /* signal handler (window resizing */
    signal(SIGWINCH, change_size);

    /* connect to server */
    msg_connect(&player_info);

    pthread_t cli_receiver;
    pthread_create(&cli_receiver, NULL, receiver, NULL);

    initscr();
    WINDOW *win_playerinfo = newwin(10, 50, 1, 0);
    keypad(win_playerinfo, TRUE);
    curs_set(0);

    print_title();

    pthread_mutex_lock(&player_data);
    while (player_info.active) {
        pthread_mutex_unlock(&player_data);

        pthread_mutex_lock(&player_data);

        /* clear player info window */
        wclear(win_playerinfo);

        wattron(win_playerinfo, A_BOLD);    /* set bold */
        if (player_info.wins != 1)          /* check what form to use (plural vs singular) */
            wprintw(win_playerinfo, "Player %d:\t%d wins\n", player_info.player_id, player_info.wins);
        else
            wprintw(win_playerinfo, "Player %d:\t%d win\n", player_info.player_id, player_info.wins);
        wattroff(win_playerinfo, A_BOLD);   /* unset bold */

        /* print player's statistics */
        wprintw(win_playerinfo, "Resources:\t%d\n", player_info.resources);
        wprintw(win_playerinfo, "Workers:\t%d\n", player_info.workers);
        wprintw(win_playerinfo, "Light inf.:\t%d\n", player_info.light_inf);
        wprintw(win_playerinfo, "Heavy inf.:\t%d\n", player_info.heavy_inf);
        wprintw(win_playerinfo, "Cavalry:\t%d\n", player_info.cavalry);

        /* refresh window */
        wrefresh(win_playerinfo);

        pthread_mutex_unlock(&player_data);
        sleep(1);   /* don't refresh too quickly */
        /* TODO: ^ odswiezanie odpiero jak dostanie info (signale z pthreadow) */

        pthread_mutex_lock(&player_data);
    }
    pthread_mutex_unlock(&player_data);

    pthread_join(cli_receiver, NULL);
    endwin();

    return 0;
}
