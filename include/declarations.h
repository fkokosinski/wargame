#ifndef WARGAME_DECLARATIONS_H
#define WARGAME_DECLARATIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <ncurses.h>
#include <sys/msg.h>

/* getopt options */
#define CHOOSE_SERVER 's'
#define CHOOSE_CLIENT 'c'
#define CHOOSE_HELP 'h'

/* number of players */
#define PLAYER_NUM 3

/* player's structure */
struct player {
    bool active;

    pid_t pid;
    unsigned int player_id;
    unsigned int resources;
    unsigned int wins;

    unsigned int workers;
    unsigned int light_inf;
    unsigned int heavy_inf;
    unsigned int cavalry;
};

#define GAME_TITLE "WARGAME"

/* standard error check */
#define CHECK_ERR(x)    if (x == -1) {          \
                            perror("wargame");  \
                            exit(EXIT_FAILURE); \
                        }

/* ping time interval */
#define PING_INTERVAL 1
#define PRODUCE_INTERVAL 1

/* recruit time */
#define WORKER_RECRUIT_TIME 2
#define LIGHT_INF_RECRUIT_TIME 2
#define HEAVY_INF_RECRUIT_TIME 3
#define CAVALRY_RECRUIT_TIME 5

/* costs */
#define WORKER_COST 150
#define LIGHT_INF_COST 100
#define HEAVY_INF_COST 250
#define CAVALRY_COST 550

/* attack stats */
#define WORKER_ATK 0.0
#define LIGHT_INF_ATK 1.0
#define HEAVY_INF_ATK 1.5
#define CAVALRY_ATK 3.5

/* defense stats */
#define WORKER_DEF 0.0
#define LIGHT_INF_DEF 1.2
#define HEAVY_INF_DEF 3.0
#define CAVALRY_DEF 1.2

#endif //WARGAME_DECLARATIONS_H
