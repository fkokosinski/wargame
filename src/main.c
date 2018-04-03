#include "declarations.h"
#include "help.h"
#include "server.h"
#include "client.h"

int main(int argc, char *argv[]) {
    int opt;
    bool server_chosen = false, client_chosen = false;

    struct option long_options[] = {
            {"server", no_argument, NULL, CHOOSE_SERVER}, /* run as server */
            {"client", no_argument, NULL, CHOOSE_CLIENT}, /* run as client */
            {"help", no_argument, NULL, CHOOSE_HELP},     /* print help */
            {0, 0, 0, 0}
    };

    while (true) {
        /* check if both options are chosen (server & client) */
        if (server_chosen && client_chosen) {
            printf("Run this program only as server or client!\n");
            exit(1);
        }

        /* else -- check the options */
        opt = getopt_long(argc, argv, "sch", long_options, NULL);
        if (opt == -1) {
            if (!server_chosen && !client_chosen)   /* print help if no args given */
                print_help(argv[0]);
            break;
        } else if (opt == 's') {
            server_chosen = true;
        } else if (opt == 'c') {
            client_chosen = true;
        } else if (opt == 'h') {
            print_help(argv[0]);
            exit(0);
        }
    }

    if (server_chosen)
        server_main();
    else if (client_chosen)
        client_main();

    return 0;
}