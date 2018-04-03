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

#endif //WARGAME_DECLARATIONS_H
