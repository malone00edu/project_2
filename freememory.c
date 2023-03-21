#define _GNU_SOURCE
#include <stdlib.h>
#include "header.h"

void free_pointers(char *cmd, char **parameters, char **instructions, int *tokenIndex) { // free ptrs after ea. instruction | exit | EOF
    for (int x = 0; x < *tokenIndex; x++) {
        free(instructions[x]);
    }
    free(instructions);
    free(parameters);
    free(cmd);
}