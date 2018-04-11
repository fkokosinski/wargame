#include "msg.h"

static int _msg_id;

/* open message queue and check for errors */
void msg_initialize(void) {
    _msg_id = msgget(QUEUE_K, IPC_CREAT | IPC_EXCL | 0640);
    if (errno == EEXIST) {
        printf("wargame: server already running or didn't close correctly\n");
        printf("make sure to close the server and write 'ipcrm -Q %d'\n", QUEUE_K);
    }
    CHECK_ERR(_msg_id);
}

/* wait for player to connect and send him his player_id */
pid_t msg_getpid(unsigned int num) {
    struct msgcli message, number;

    /* wait for new player to connect */
    int err = (int)msgrcv(_msg_id, &message, BUF_SIZE(struct msgcli), MSG_TYPE_PRE, 0);
    CHECK_ERR(err);

    /* send him his number */
    number.mtype = message.pid;
    number.player_num = num;
    err = msgsnd(_msg_id, &number, BUF_SIZE(struct msgcli), 0);
    CHECK_ERR(err);

    /* return his pid */
    return message.pid;
}

void msg_notify(pid_t pid) {
    struct msgcli message = {
            .mtype = pid
    };

    int err = msgsnd(_msg_id, &message, BUF_SIZE(struct msgcli), 0);
    CHECK_ERR(err);
}

void msg_close(void) {
    msgctl(_msg_id, IPC_RMID, NULL);
}

bool msg_ping(struct player player_data) {
    struct msgserv message = {
            .mtype = player_data.pid,
            .player_data = player_data,
            .sectype = SECTYPE_PING
    };
    struct msgcli pong;

    /* send ping */
    int err = msgsnd(_msg_id, &message, BUF_SIZE(struct msgserv), 0);
    CHECK_ERR(err);

    /* wait for response */
    sleep(PING_INTERVAL);
    err = (int)msgrcv(_msg_id, &pong, BUF_SIZE(struct msgcli), MSG_TYPE_PING(player_data.player_id), IPC_NOWAIT);

    if (err == -1 && errno == ENOMSG)
        return false;

    CHECK_ERR(err);
    return true;
}

bool msg_pong(struct player *player_data) {
    struct msgserv message;
    struct msgcli response = {
            .mtype = MSG_TYPE_PING(player_data->player_id)
    };

    /* check if any messages for client */
    int err = (int)msgrcv(_msg_id, &message, BUF_SIZE(struct msgserv), MSG_TYPE_CLIENT, IPC_NOWAIT);
    if (err == -1 && errno == ENOMSG) { /* if not -- return false */
        return false;
    }
    CHECK_ERR(err);

    /* if ping -- send response */
    if (message.sectype == SECTYPE_PING) {
        *player_data = message.player_data;
        err = msgsnd(_msg_id, &response, BUF_SIZE(struct msgcli), 0);
        CHECK_ERR(err);
    }

    return true;
}

struct recruit_req msg_recruit(int player_id) {
    struct msgcli message;
    struct recruit_req request;

    /* wait for request */
    int err = (int)msgrcv(_msg_id, &message, BUF_SIZE(struct msgcli), MSG_TYPE_RECRUIT(player_id), 0);
    CHECK_ERR(err);

    request.unit = message.unit;
    request.quantity = message.quantity;

    return request;
}

void msg_request(int player_id, int unit, int quantity) {
    struct msgcli message = {
            .mtype = MSG_TYPE_RECRUIT(player_id),
            .unit = unit,
            .quantity = quantity
    };

    /* send request */
    int err = msgsnd(_msg_id, &message, BUF_SIZE(struct msgcli), 0);
    CHECK_ERR(err);
}

void msg_attack(int source, int target, struct atk_party units) {
    struct msgcli message = {
            .mtype = MSG_TYPE_ATTACK(source),
            .attack = units
    };
    message.attack.source = source;
    message.attack.target = target;

    int err = msgsnd(_msg_id, &message, BUF_SIZE(struct msgcli), 0);
    CHECK_ERR(err);
}

struct atk_party msg_atk_fetch(int player_id) {
    struct msgcli message;

    int err = (int)msgrcv(_msg_id, &message, BUF_SIZE(struct msgcli), MSG_TYPE_ATTACK(player_id), 0);
    CHECK_ERR(err);

    return message.attack;
}

void msg_connect(struct player *player_data) {
    /* open message queue and check for errors */
    _msg_id = msgget(QUEUE_K, 0);

    if (errno == ENOENT) {
        printf("wargame: make sure to run server first!\n");
    }
    CHECK_ERR(_msg_id);

    struct msgcli initial_msg = {
            .mtype = MSG_TYPE_PRE,
            .pid = getpid()
    };

    struct msgcli number;

    /* send initial message to server */
    int err = msgsnd(_msg_id, &initial_msg, BUF_SIZE(struct msgcli), 0);
    CHECK_ERR(err);

    /* retrieve player number */
    err = (int)msgrcv(_msg_id, &number, BUF_SIZE(struct msgcli), MSG_TYPE_CLIENT, 0);
    CHECK_ERR(err);
    player_data->player_id = number.player_num;
    player_data->active = true;
    player_data->pid = getpid();

    /* wait for game start */
    err = (int)msgrcv(_msg_id, &number, BUF_SIZE(struct msgcli), MSG_TYPE_CLIENT, 0);
    CHECK_ERR(err);
}
