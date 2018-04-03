#include "server.h"

struct player player_arr[PLAYER_NUM];
static pthread_mutex_t player_data = PTHREAD_MUTEX_INITIALIZER;

/* produce players' resources
 * argument: ptr to player's id */
void *resource_producer(void *data) {
    int player_id = *(int*)data;

    pthread_mutex_lock(&player_data);
    while (player_arr[player_id].active) {
        /* produce... */
        printf("[P%d %d] Resources: %d ", player_arr[player_id].player_id, player_arr[player_id].pid, player_arr[player_id].resources);
        player_arr[player_id].resources += 50 + player_arr[player_id].workers * 5;
        printf("-> %d\n", player_arr[player_id].resources);

        /* ... and wait */
        pthread_mutex_unlock(&player_data);
        sleep(PRODUCE_INTERVAL);
        pthread_mutex_lock(&player_data);
    }
    pthread_mutex_unlock(&player_data);

    return NULL;
}

/* ping player: send a message to queue and wait PING_INTERVAL seconds; check if there's answer - if not, the player
 * has lost connection
 * argument: ptr to player's id*/
void *pinger(void *data) {
    int player_id = *(int*)data;

    pthread_mutex_lock(&player_data);
    pid_t pid = player_arr[player_id].pid;
    pthread_mutex_unlock(&player_data);

    struct player player_data_send;

    pthread_mutex_lock(&player_data);
    while (player_arr[player_id].active) {
        /* fetch player's data to send */
        player_data_send = player_arr[player_id];
        pthread_mutex_unlock(&player_data);

        /* send ping and wait for response */
        printf("[P%d %d] Player pinged\n", player_id, pid);
        if (msg_ping(player_data_send)) {   /* if responded */
            printf("[P%d %d] PONG\n", player_id, pid);
        } else {
            printf("[P%d %d] Connection lost\n", player_id, pid);
            pthread_mutex_lock(&player_data);
            player_arr[player_id].active = false;
            pthread_mutex_unlock(&player_data);
        }

        pthread_mutex_lock(&player_data);
    }
    pthread_mutex_unlock(&player_data);

    return NULL;
}

int server_main(void) {
    msg_initialize();

    /* initial phase -- wait for all players to join */
    for (unsigned int i = 0; i < PLAYER_NUM; i++) {
        /* wait for new player to connect */
        player_arr[i].pid = msg_getpid(i);
        player_arr[i].active = true;
        player_arr[i].player_id = i;
        player_arr[i].wins = 0;

        player_arr[i].resources = 300;
        player_arr[i].workers = 0;
        player_arr[i].light_inf = 0;
        player_arr[i].heavy_inf = 0;
        player_arr[i].cavalry = 0;

        printf("Player %d connected.\n", i);
    }

    /* send info about game start */
    for (int i = 0; i < PLAYER_NUM; i++)
        msg_notify(player_arr[i].pid);

    pthread_t player_producer[3], player_pinger[3];
    for (int i = 0; i < PLAYER_NUM; i++) {
        pthread_mutex_lock(&player_data);
        pthread_create(&player_producer[i], NULL, resource_producer, &player_arr[i].player_id);
        pthread_create(&player_pinger[i], NULL, pinger, &player_arr[i].player_id);
        pthread_mutex_unlock(&player_data);
    }

    pthread_join(player_producer[0], NULL);

    msg_close();
    return 0;
}
