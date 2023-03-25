#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include "header.h"

void find_path(char **instr, bool *validExecution, int *tokenIndex) {

    char *arrOfDirectories[6];
    arrOfDirectories[0] = "/usr/local/sbin", arrOfDirectories[1] = "/usr/local/bin",
    arrOfDirectories[2] = "/usr/sbin", arrOfDirectories[3] = "/usr/bin",
    arrOfDirectories[4] = "/sbin", arrOfDirectories[5] = "/bin";
    size_t size = (strlen(arrOfDirectories[0]) + strlen(&instr[0][0]) + 5);

    char *location = malloc(size * sizeof(char));
    memset(location, 0, size * sizeof(char));
    struct stat stats;
    errno = 0;
    bool found = false;
    for (int i = 0; i < 6; i++) {
        strcpy(location, arrOfDirectories[i]); // cycle through possible directories
        strcat(location, "/");
        strcat(location, &instr[0][0]); // instr[0] is only holding files: ls, pwd, where, cat, etc, in this funct.
        stat(location, &stats); // check to see if file exists with created path. ex: location = /bin/ls
        if (stat(location, &stats) == 0) {
            instr[0] = strdup(location);
            //printf("%s\n", instr[0]);
            found = true;
            break;
        }
    }
    if (found == false) {
        errno = ENOENT;
        printf("File does not exist within bin directories! (%s)\n", strerror(errno));
        *validExecution = false;
        free(location);
        return;
    }
    free(location);
}

void fork_adv(char **instructions, bool *validExecution, int *tokenIndex) {
    char ***arrForPipeRedirect = NULL; // 2D Array
    size_t symbolsFound = 0;
    //size_t pipeSymbolPos = 0;
    size_t commandsAndParameters = 0;
    bool pipeSymbolFound = false;
    bool redirectSymbolFound = false;

    for (int i = 0; i < *tokenIndex; i++) {
        if ((strcmp(instructions[i], "<") == 0) | (strcmp(instructions[i], ">") == 0)) {
            redirectSymbolFound = true;
            symbolsFound++;
        }
        if ((strcmp(instructions[i], "|") == 0)) {
            pipeSymbolFound = true;
            //pipeSymbolPos = i;
            symbolsFound++;
        }
    }

    if (redirectSymbolFound | pipeSymbolFound) {
         commandsAndParameters = *tokenIndex - symbolsFound;
        arrForPipeRedirect = malloc((symbolsFound + 1) * sizeof(char **)); // Rows
        for (int i = 0; i < symbolsFound + 1; i++) {
            arrForPipeRedirect[i] = NULL;
        }
        for (int i = 0; i < symbolsFound + 1; i++) {
            arrForPipeRedirect[i] = calloc(commandsAndParameters, sizeof(char *)); // Columns
        }
        int index = 0;
        int rowPos = 0;
        int colPos = 0;
        while (index < *tokenIndex) { // Filling out a 2D array. Breaks to next row when "<", ">", or "|" is found.
            if (strcmp(instructions[index], "|") != 0) {
                arrForPipeRedirect[rowPos][colPos] = strdup(instructions[index]);
                //printf("%s\n", arrForPipeRedirect[rowPos][colPos]);
                colPos++;
            } else {
                rowPos++;
                colPos = 0;
            }
            index++;
        }
    }

    /*
     * 2D Array example for "ls -1 | wc -l"
     *
     *    0: 1: 2:
     * 0: ls -1
     * 1: wc -l
     * 2:
     *
     *
     * */

    //char *arr1[] = {"ls", "-1", NULL};
    //char *arr2[] = {"wc", "-l", NULL};
    find_path(arrForPipeRedirect[0], validExecution, tokenIndex);
    find_path(arrForPipeRedirect[1], validExecution, tokenIndex);
    //printf("%s\n", arrForPipeRedirect[0][0]);
    //printf("%s\n", arrForPipeRedirect[1][0]);
    //return;

    char *cmd1, *cmd2;
    char *parm1, *parm2;
    int fd[2];
    pid_t pid;

    if (pipe(fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        execv(arrForPipeRedirect[0][0], arrForPipeRedirect[0]);
        perror("execv");
        exit(EXIT_FAILURE);
    } else {
        wait(NULL);
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close((fd[0]));
        execv(arrForPipeRedirect[1][0], arrForPipeRedirect[1]);
        perror("execv");
        exit(EXIT_FAILURE);
    }
}

/*if (strchr(curr_line(), '|') == 0) {
    return;
} else if ((strcmp(instructions[1], "<") == 0) | (strcmp(instructions[1], ">") == 0)) {
}*/

/*int cpid = fork();
if (cpid > 0) { // parent
    wait(NULL); // wait for child
} else if (cpid == 0) {
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
}*/