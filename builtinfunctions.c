#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "header.h"

void builtin_cd(char *const *instructions) {
    char *dir = instructions[1];
    if (dir != NULL) {
        if (strncmp(dir, "~/", 2) == 0) {
            // replace the tilde with the value of HOME
            char *home = getenv("HOME");
            if (home == NULL) {
                printf("!mysh> HOME environment variable missing\n");
            } else {
                char newdir[strlen(home) + strlen(dir) - 1];
                sprintf(newdir, "%s%s", home, dir + 1);
                if (chdir(newdir) != 0) {
                    printf("!mysh> No such file or directory.\n");
                }
            }
        } else {
            if (chdir(dir) != 0) {
                printf("!mysh> No such file or directory.\n");
            }
        }
    } else {
        char *home = getenv("HOME");
        if (home == NULL) {
            printf("!mysh> HOME environment variable missing.\n");
        } else {
            if (chdir(home) != 0) {
                printf("!mysh> No such file or directory.\n");
            }
        }
    }
}

void builtin_pwd(){}