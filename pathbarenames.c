#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "header.h"

void use_fork_basic(char *cmd, char **parameters, char *const *instructions, bool *validExecution) {
    errno = 0;
    int id = fork();
    if (id > 0) { // parent
        wait(NULL); // wait for child
    } else if (id == 0) {
        strcpy(cmd, instructions[0]); // instr[0] is holding the path + file, in this funct.
        execv(cmd, parameters);  // execute command
        if (errno != 0) {
            printf("Failed to execute command! (%s)\n", strerror(errno));
            *validExecution = false;
        }
    } else {
        printf("Failed to create a child process! (%s)\n", strerror(errno));
    }
}

void use_fork_adv(char *cmd, char **parameters, char *const *instructions,
                  char **arrOfDirectories, bool *validExecution) {
    size_t size = (strlen(arrOfDirectories[0]) + strlen(instructions[0]) + 5);
    char *location = malloc( size * sizeof(char));
    memset(location, 0, size * sizeof(char));
    char *path;
    struct stat stats;
    errno = 0;
    bool found = false;
    for (int i = 0; i < 6; i++) {
        strcpy(location, arrOfDirectories[i]); // cycle through possible directories
        strcat(location, "/");
        path = strdup(location); // Possible path to the file contained in instructions[0]. Needed for execv()
        strcat(location, instructions[0]); // instr[0] is only holding files: ls, pwd, where, cat, etc, in this funct.
        stat(location, &stats); // check to see if file exists with created path. ex: location = /bin/ls
        if (stat(location, &stats) == 0) {
            found = true;
            break;
        } else {
            free(path);
        }
    }
    if (found == false) {
        errno = ENOENT;
        printf("File does not exist within bin directories! (%s)\n", strerror(errno));
        *validExecution = false;
        free(location);
        return;
    } else {
        int id = fork();
        if (id > 0) { // parent
            wait(NULL); // wait for child
        } else if (id == 0) {
            strcpy(cmd, path); // ex: /bin/
            strcat(cmd, instructions[0]); // ex: ls
            execv(cmd, parameters);
            if (errno != 0) {
                printf("Failed to execute command! (%s)\n", strerror(errno));
                *validExecution = false;
            }
        } else {
            printf("Failed to create a child process! (%s)\n", strerror(errno));
        }
    }
    free(path);
    free(location);
}