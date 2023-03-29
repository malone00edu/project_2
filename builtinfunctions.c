#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "header.h"

void builtin_cd(char *const *instructions) {
    char *dir = instructions[1];
    if (dir == NULL) {
        dir = getenv("HOME");
        if (dir == NULL) {
            prompt_homeerr();
            return;
        }
    }
    else if (dir[0] == '~') {
        char *home = getenv("HOME");
        if (home == NULL) {
            prompt_homeerr();
            return;
        }
        char newdir[strlen(home) + strlen(dir)];
        sprintf(newdir, "%s%s", home, dir+1);
        dir = newdir;
    }

    if (chdir(dir) != 0) {
        prompt_nodir();
    }
}

void builtin_pwd() {
    char current[2000];
    memset(current, 0, 2000 * sizeof(char));
    if (getcwd(current, sizeof(current)) != NULL) {
        printf("%s\n", current);
    } else {
        perror("!mysh> error w getcwd");
    }
}
