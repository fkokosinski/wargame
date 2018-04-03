#include "help.h"

void print_help(const char prog_name[]) {
    printf("Usage:\n");
    printf("%s [--client|-c] -- run as client\n", prog_name);
    printf("%s [--server|-s] -- run as server\n", prog_name);
    printf("%s [--help|-h] -- show this information and exit\n", prog_name);
}