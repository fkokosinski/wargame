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

void *attacker(void *data) {
    int player_id = *(int*)data;
    struct atk_party attack;
    float power_atk, power_def, ratio;
    int target;

    pthread_mutex_lock(&player_data);
    while (player_arr[player_id].active) {
        pthread_mutex_unlock(&player_data);

        attack = msg_atk_fetch(player_id);

        /* check if has enough units */
        pthread_mutex_lock(&player_data);
        if (player_arr[player_id].light_inf < attack.light_inf)
            continue;
        if (player_arr[player_id].heavy_inf < attack.heavy_inf)
            continue;
        if (player_arr[player_id].cavalry < attack.cavalry)
            continue;

        /* if has -- take these units from his counts */
        player_arr[player_id].light_inf -= attack.light_inf;
        player_arr[player_id].heavy_inf -= attack.heavy_inf;
        player_arr[player_id].cavalry -= attack.cavalry;

        /* check who has more atk points */
        target = attack.target;
        power_atk = attack.light_inf + attack.heavy_inf + attack.cavalry;
        power_def = player_arr[target].light_inf + player_arr[target].heavy_inf + player_arr[target].cavalry;

        if (power_atk - power_def > 0.0) {    /* attacker has won */
            player_arr[player_id].wins++;

            /* kill all defender's units */
            player_arr[target].workers = 0;
            player_arr[target].light_inf = 0;
            player_arr[target].heavy_inf = 0;
            player_arr[target].cavalry = 0;

            /* kill some of attacker's units */
            ratio = power_def / power_atk;
            attack.light_inf -= attack.light_inf * ratio;
            attack.heavy_inf -= attack.heavy_inf * ratio;
            attack.cavalry -= attack.cavalry * ratio;
        } else {
            ratio = power_atk / power_def;

            /* kill some of defender's units */
            player_arr[target].workers -= player_arr[target].workers * ratio;
            player_arr[target].light_inf -= player_arr[target].light_inf * ratio;
            player_arr[target].heavy_inf -= player_arr[target].heavy_inf * ratio;
            player_arr[target].cavalry -= player_arr[target].cavalry * ratio;

            /* kill some of attacker's units */
            attack.light_inf -= attack.light_inf * ratio;
            attack.heavy_inf -= attack.heavy_inf * ratio;
            attack.cavalry -= attack.cavalry * ratio;
        }

        /* return units */
        player_arr[player_id].light_inf += attack.light_inf;
        player_arr[player_id].heavy_inf += attack.heavy_inf;
        player_arr[player_id].cavalry += attack.cavalry;

        if (player_arr[player_id].wins == 3)
            for (int i = 0; i < PLAYER_NUM; i++)
                player_arr[i].active = false;

        pthread_mutex_unlock(&player_data);
        pthread_mutex_lock(&player_data);
    }
    pthread_mutex_unlock(&player_data);

    return NULL;
}

void *recruiter(void *data) {
    int player_id = *(int*)data;
    struct recruit_req request;

    pthread_mutex_lock(&player_data);
    while (player_arr[player_id].active) {
        pthread_mutex_unlock(&player_data);

        /* wait for request */
        request = msg_recruit(player_id);

        /* check if enough resources */
        pthread_mutex_lock(&player_data);
        if (request.unit == 0 && request.quantity*WORKER_COST > player_arr[player_id].resources)
            continue;
        else if (request.unit == 1 && request.quantity*LIGHT_INF_COST > player_arr[player_id].resources)
            continue;
        else if (request.unit == 2 && request.quantity*HEAVY_INF_COST > player_arr[player_id].resources)
            continue;
        else if (request.unit == 3 && request.quantity*CAVALRY_COST > player_arr[player_id].resources)
            continue;
        pthread_mutex_unlock(&player_data);

        /* check if enough resources */
        pthread_mutex_lock(&player_data);
        if (request.unit == 0)
            player_arr[player_id].resources -= WORKER_COST*request.quantity;
        else if (request.unit == 1)
            player_arr[player_id].resources -= LIGHT_INF_COST*request.quantity;
        else if (request.unit == 2)
            player_arr[player_id].resources -= HEAVY_INF_COST*request.quantity;
        else if (request.unit == 3)
            player_arr[player_id].resources -= CAVALRY_COST*request.quantity;
        pthread_mutex_unlock(&player_data);

        /* recruit */
        for (int i = 0; i < request.quantity; i++) {
            if (request.unit == 0) {
                sleep(WORKER_RECRUIT_TIME);
                pthread_mutex_lock(&player_data);
                player_arr[player_id].workers++;
                pthread_mutex_unlock(&player_data);
            } else if (request.unit == 1) {
                sleep(LIGHT_INF_RECRUIT_TIME);
                pthread_mutex_lock(&player_data);
                player_arr[player_id].light_inf++;
                pthread_mutex_unlock(&player_data);
            } else if (request.unit == 2) {
                sleep(HEAVY_INF_RECRUIT_TIME);
                pthread_mutex_lock(&player_data);
                player_arr[player_id].heavy_inf++;
                pthread_mutex_unlock(&player_data);
            } else if (request.unit == 3) {
                sleep(CAVALRY_RECRUIT_TIME);
                pthread_mutex_lock(&player_data);
                player_arr[player_id].heavy_inf++;
                pthread_mutex_unlock(&player_data);
            }
        }
    }

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

    pthread_t player_producer[PLAYER_NUM], player_pinger[PLAYER_NUM], player_recruiter[PLAYER_NUM], player_atk[PLAYER_NUM];
    for (int i = 0; i < PLAYER_NUM; i++) {
        pthread_mutex_lock(&player_data);
        pthread_create(&player_producer[i], NULL, resource_producer, &player_arr[i].player_id);
        pthread_create(&player_pinger[i], NULL, pinger, &player_arr[i].player_id);
        pthread_create(&player_recruiter[i], NULL, recruiter, &player_arr[i].player_id);
        pthread_create(&player_atk[i], NULL, attacker, &player_arr[i].player_id);
        pthread_mutex_unlock(&player_data);
    }

    pthread_join(player_producer[0], NULL);

    msg_close();
    return 0;
}
