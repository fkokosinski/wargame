#ifndef WARGAME_MSG_H
#define WARGAME_MSG_H

#include "declarations.h"

/* message buffer size */
#define BUF_SIZE(x) (((sizeof(x)) - (sizeof(long))))

/* message queue key */
#define QUEUE_K 12345

/* message buffer types */
#define MSG_TYPE_PRE 1              /* used for pre-game communication */
#define MSG_TYPE_PING(x) ((x)+2)    /* used for sending pings server->client */
#define MSG_TYPE_RECRUIT(x) ((x)+5) /* used for sending recruit request client->server */
#define MSG_TYPE_CLIENT getpid()    /* retrieve info as client */

/* message buffer types (secondary) for server->client communication */
#define SECTYPE_PING 0

/* message queue buffer for communication client->server */
struct msgcli {
    long mtype;
    pid_t pid;
    unsigned int player_num;

    int unit;
    int quantity;
};

/* used for pinging server->client */
struct msgserv {
    long mtype;
    int sectype;
    struct player player_data;
};

/* recruit request */
struct recruit_req {
    int unit;
    int quantity;
};

void msg_initialize(void);
pid_t msg_getpid(unsigned int num);
void msg_notify(pid_t pid);
void msg_close(void);
bool msg_ping(struct player player_data);
bool msg_pong(struct player *player_data);
void msg_connect(struct player *player_data);
struct recruit_req msg_recruit(int player_id);
void msg_request(int player_id, int unit, int quantity);

#endif
