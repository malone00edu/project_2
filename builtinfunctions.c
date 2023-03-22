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
        if (chdir(dir) != 0) {
            printf("!mysh> No such file or directory.\n");
        }
    } 
    else {
            printf("!mysh> No such file or directory.\n");
        }
}


    /*
    
    char *dir = instructions[1];
    if (dir != NULL) {
        if (strncmp(dir, "~/", 2) == 0) {
            char *home = getenv("HOME");
            if (home == NULL) {
                printf("!mysh> HOME environment variable missing.\n");
                return;
            }
            char newdir[strlen(home) + strlen(dir) - 1];
            sprintf(newdir, "%s%s", home, dir + 1);
            if (chdir(newdir) != 0) {
                printf("!mysh> No such file or directory.\n");
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
            return;
        }
        if (chdir(home) != 0) {
            printf("!mysh> No such file or directory.\n");
        }
    }
}
*/



void builtin_pwd(){
    char current[2000];
    if (getcwd(current, sizeof(current)) != NULL) {
        printf("%s\n", current);
    } else {
        perror("!mysh> error w getcwd");
    }
}
