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

void is_indirect(char ***arrForPipeRedirect, int iRow, int indirect, int din, bool *validExecution);

void is_outdirect(char ***arrForPipeRedirect, int iRow, int outdirect, int dout, bool *validExecution);

void find_path(char **instr, bool *validExecution) {

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

    char ***pipeRedirectCommands = NULL; // 2D Array
    size_t symbolsFound = 0;
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
            symbolsFound++;
        }
    }

    if (redirectSymbolFound | pipeSymbolFound) {
        commandsAndParameters = *tokenIndex - symbolsFound;
        pipeRedirectCommands = malloc((symbolsFound + 1) * sizeof(char **)); // create rows
        for (int i = 0; i < symbolsFound + 1; i++) {
            pipeRedirectCommands[i] = NULL;
        }
        for (int i = 0; i < symbolsFound + 1; i++) {
            pipeRedirectCommands[i] = calloc(commandsAndParameters, sizeof(char *)); // create columns
        }
        int index = 0;
        int rowPos = 0;
        int colPos = 0;
        while (index < *tokenIndex) { // Filling out a 2D array. Advances to next row when "|" is found.
            if (strcmp(instructions[index], "|") != 0) {
                pipeRedirectCommands[rowPos][colPos] = strdup(instructions[index]);
                colPos++;
            } else {
                rowPos++;
                colPos = 0;
            }
            index++;
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

        /*
         * 2D Array example for "ls -1 | wc -l > out.txt"
         *
         *    0: 1: 2:
         * 0: ls -1
         * 1: wc -l > out.txt
         * 2:
         *
         *
         * */

        find_path(pipeRedirectCommands[0], validExecution);
        find_path(pipeRedirectCommands[1], validExecution);

        int indirect = 0;
        int outdirect = 0;
        int din = 0;
        int dout = 0;
        for (int iRow = 0; iRow < symbolsFound + 1; iRow++) {
            for (int iCol = 0; iCol < commandsAndParameters; iCol++) {
                if (pipeRedirectCommands[iRow][iCol] != NULL) {
                    if (strcmp(pipeRedirectCommands[iRow][iCol], "<") == 0) {
                        indirect = iCol + 1;
                        if (indirect) {
                            // your indirect is called. see end of this file
                            is_indirect(pipeRedirectCommands, iRow, indirect, din, NULL);
                        }
                    } else if (strcmp(pipeRedirectCommands[iRow][iCol], ">") == 0) {
                        outdirect = iCol + 1;
                        if (outdirect) {
                            // your outdirect is called. see end of this file
                            is_outdirect(pipeRedirectCommands, iRow, outdirect, dout, NULL);
                        }
                    }
                }
            }
        }

        int fd[2];
        errno = 0;
        if (pipe(fd) == -1) { // create pipe
            printf("Pipe() failure! (%s)\n", strerror(errno));
            *validExecution = false;
            //return;
        }
        pid_t pid1 = fork();
        if (pid1 == -1) { // create fork
            printf("Fork() failure! (%s)\n", strerror(errno));
            *validExecution = false;
        } else if (pid1 == 0) { // child 1
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            close(fd[1]);
            execv(pipeRedirectCommands[0][0], pipeRedirectCommands[0]);

            if (errno != 0) {
                printf("Failed to execute command! (%s)\n", strerror(errno));
                *validExecution = false;
                // return;
            }
        }

        pid_t pid2 = fork();
        if (pid2 == -1) {
            printf("Fork() failure! (%s)\n", strerror(errno));
            *validExecution = false;
        } else if (pid2 == 0) {
            dup2(fd[0], STDIN_FILENO);
            close(fd[1]);
            close(fd[0]);
            execv(pipeRedirectCommands[1][0], pipeRedirectCommands[1]);

            if (errno != 0) {
                printf("Failed to execute command! (%s)\n", strerror(errno));
                *validExecution = false;
                //return;
            }
        }
        close(fd[0]);
        close(fd[1]);
        wait(0);
        wait(0);

    } else {

        find_path(instructions, validExecution);

        char *cmd = NULL;
        int cpid = fork();
        if (cpid > 0) { // parent
            wait(NULL); // wait for child
        } else if (cpid == 0) {
            cmd = strdup(instructions[0]); // ex: /bin/
            execv(cmd, instructions);
            if (errno != 0) {
                printf("Failed to execute command! (%s)\n", strerror(errno));
                *validExecution = false;
            }
            free(cmd);
        } else {
            printf("Failed to create a child process! (%s)\n", strerror(errno));
        }
    }
}


void is_indirect(char ***arrForPipeRedirect, int iRow, int indirect, int din, bool *validExecution) {
    din = open(arrForPipeRedirect[iRow][indirect], O_RDONLY);
    if (din == -1) {
        printf("Failed to open file! (%s)\n", strerror(errno));
        *validExecution = false;
        //return;
    }
    dup2(din, STDIN_FILENO);
    close(din);
    arrForPipeRedirect[iRow][indirect - 1] = NULL;
}

void is_outdirect(char ***arrForPipeRedirect, int iRow, int outdirect, int dout, bool *validExecution) {
    dout = open(arrForPipeRedirect[iRow][outdirect], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
    if (dout == -1) {
        printf("Failed to to open file! (%s)\n", strerror(errno));
        *validExecution = false;
        //return;
    }
    dup2(dout, STDOUT_FILENO);
    close(dout);
    arrForPipeRedirect[iRow][outdirect - 1] = NULL;
}
