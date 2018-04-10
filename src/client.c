#include "client.h"

struct player player_info;
static pthread_mutex_t player_data = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t new_info;

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
        pthread_cond_signal(&new_info);

        pthread_mutex_lock(&player_data);
    }
    pthread_mutex_unlock(&player_data);

    return NULL;
}

void *print_info(void *data) {

    pthread_mutex_lock(&player_data);
    while (player_info.active) {
        pthread_mutex_unlock(&player_data);

        pthread_mutex_lock(&player_data);
        pthread_cond_wait(&new_info, &player_data);
        print_playerinfo(player_info);
        pthread_mutex_unlock(&player_data);

        pthread_mutex_lock(&player_data);
    }

    return NULL;
}

int client_main(void) {
    char str[INPUT_LEN];

    /* signal handler (window resizing */
    signal(SIGWINCH, tui_resize);

    /* connect to server */
    msg_connect(&player_info);

    pthread_t cli_receiver, tui_info;
    pthread_create(&cli_receiver, NULL, receiver, NULL);

    tui_ctl(TUI_INIT);
    pthread_create(&tui_info, NULL, print_info, NULL);
    print_title();

    pthread_mutex_lock(&player_data);
    while (player_info.active) {
        pthread_mutex_unlock(&player_data);

        /* get command */
        mvgetnstr(LINES-1, 0, str, INPUT_LEN);
        pthread_mutex_lock(&player_data);
        wclear(stdscr);
        print_title();
        print_playerinfo(player_info);
        pthread_mutex_unlock(&player_data);

        /* process accordingly */
        if (str[0] == 'r') {    /* recruit units */
            msg_request(player_info.player_id, atoi(&str[2]), atoi(&str[4]));
        }

        pthread_mutex_lock(&player_data);
    }
    pthread_mutex_unlock(&player_data);

    pthread_join(cli_receiver, NULL);

    tui_ctl(TUI_EXIT);
    return 0;
}
