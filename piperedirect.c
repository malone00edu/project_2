#define _GNU_SOURCE

void seek_wildcard(char **instructions);

#include "header.h"

void execute_pipe_redirect(char **instructions, int *tokenIndex, bool *validExecution) {

    char ***arguments = NULL; // 2D Array
    static char *cmd1;
    static char *cmd2;
    size_t symbolsFound = 0;
    bool pipeSymbolFound = false;
    bool redirectSymbolFound = false;
    int indirect = 0;
    int outdirect = 0;
    int din = 0;
    int dout = 0;

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
    seek_wildcard(instructions);
    create_2d_array(instructions, tokenIndex, validExecution, symbolsFound, &arguments, &cmd1, &cmd2);

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

    if (pipeSymbolFound) { // If a pipe is found. Use this.
        int fd[2];
        errno = 0;
        if (pipe(fd) == -1) { // create pipe
            printf("Pipe() failure! (%s)\n", strerror(errno));
            *validExecution = false;
            return;
        }
        pid_t pid1 = fork();
        if (pid1 == -1) { // create fork
            printf("Fork() failure! (%s)\n", strerror(errno));
            *validExecution = false;
        } else if (pid1 == 0) { // child 1

            dup2(fd[1], STDOUT_FILENO);
            close(fd[1]);
            close(fd[0]);
            if (redirectSymbolFound) {
                use_redirection(validExecution, arguments, symbolsFound, tokenIndex,
                                indirect, outdirect, din, dout);
            }
            execv(cmd1, *arguments);
            if (errno != 0) {
                printf("Failed to execute command! (%s)\n", strerror(errno));
                *validExecution = false;
                return;
            }
        }
        pid_t pid2 = fork();
        if (pid2 == -1) {
            printf("Fork() failure! (%s)\n", strerror(errno));
            *validExecution = false;
        } else if (pid2 == 0) {
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
            close(fd[1]);

            if (redirectSymbolFound) {
                use_redirection(validExecution, arguments, symbolsFound, tokenIndex,
                                indirect, outdirect, din, dout);
            }
            execv(cmd2, *(arguments + 1));
            if (errno != 0) {
                printf("Failed to execute command! (%s)\n", strerror(errno));
                *validExecution = false;
                return;
            }
        }
        close(fd[0]);
        close(fd[1]);
        wait(0);
        wait(0);
    } else { // Else execute this part for redirects or single commands (With or without args).
        int pid = fork();
        if (pid > 0) {
            wait(NULL);
        } else if (pid == 0) {
            if (redirectSymbolFound) {
                use_redirection(validExecution, arguments, symbolsFound, tokenIndex,
                                indirect, outdirect, din, dout);
            }
            execv(cmd1, *arguments);
            if (errno != 0) {
                printf("Failed to execute command! (%s)\n", strerror(errno));
                *validExecution = false;
            }
        }
    }
    for (int i = 0; i < symbolsFound + 1; i++) {
        for (int j = 0; j < *tokenIndex + 1; j++) {
            free(arguments[i][j]);
        }
        free(arguments[i]);
    }
    free(arguments);
    free(cmd1);
    if (pipeSymbolFound) {
        free(cmd2);
    }
}

void seek_wildcard(char **instructions) {
    for (int i = 0; instructions[i] != NULL; i++) {
        if (strchr(instructions[i], '*') != NULL) {
            glob_t paths;
            glob(instructions[i], GLOB_NOCHECK | GLOB_TILDE, NULL, &paths);
            for (int j = 0; j < paths.gl_pathc; j++) {
                free(instructions[i]);
                instructions[i] = strdup(paths.gl_pathv[j]);
            }
            globfree(&paths);
        }
    }
}

void find_path(char **cmd, bool *validExecution) {

    char *arrOfDirectories[6];
    arrOfDirectories[0] = "/usr/local/sbin", arrOfDirectories[1] = "/usr/local/bin",
    arrOfDirectories[2] = "/usr/sbin", arrOfDirectories[3] = "/usr/bin",
    arrOfDirectories[4] = "/sbin", arrOfDirectories[5] = "/bin";
    size_t size = (strlen(arrOfDirectories[0]) + strlen(*cmd) + 5);

    char *location = malloc(size * sizeof(char));
    memset(location, 0, size * sizeof(char));
    struct stat stats;
    errno = 0;
    bool found = false;
    for (int i = 0; i < 6; i++) {
        strcpy(location, arrOfDirectories[i]); // cycle through possible directories
        strcat(location, "/");
        strcat(location, *cmd); // instr[0] is only holding files: ls, pwd, where, cat, etc, in this funct.)
        stat(location, &stats); // check to see if file exists with created path. ex: location = /bin/ls
        if (stat(location, &stats) == 0) {
            free(*cmd); // cmd got here with strdup(). Which has builtin malloc. Gotta free it before reuse.
            *cmd = strdup(location);
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


void create_2d_array(char *const *instructions, const int *tokenIndex, bool *validExecution, size_t symbolsFound,
                     char ****arguments, char **cmd1, char **cmd2) {
    (*arguments) = malloc((symbolsFound + 1) * sizeof(char **)); // create rows
    for (int i = 0; i < symbolsFound + 1; i++) {
        (*arguments)[i] = NULL;
    }
    for (int i = 0; i < symbolsFound + 1; i++) {
        (*arguments)[i] = calloc(*tokenIndex + 1, sizeof(char *)); // create columns
    }
    int index = 0;
    int rowPos = 0;
    int colPos = 0;
    while (index < *tokenIndex) { // Filling out a 2D array. Advances to next row when "|" is found
        if (index == 0) { // 1st cmd in instructions array
            (*arguments)[rowPos][colPos] = strdup(instructions[index]); // args with no path attached
            (*cmd1) = strdup((*arguments)[rowPos][colPos]); // ex: "/bin/ls"
            find_path(cmd1, validExecution);
            colPos++;
        }
        if (index > 0) { // fill out first row until "|" is encountered.
            if (strcmp(instructions[index], "|") != 0) {
                (*arguments)[rowPos][colPos] = strdup(instructions[index]);
                colPos++;
            } else { // after '|' is encountered. we end up here. adv to next row. set col = 0. adv index and fill
                rowPos++;
                colPos = 0;
                index++;
                (*arguments)[rowPos][colPos] = strdup(instructions[index]); // args with no path attached
                (*cmd2) = strdup((*arguments)[rowPos][colPos]); // ex: : "/bin/wc". After a '|', there must be a cmd
                find_path(cmd2, validExecution);
                colPos++;
            }
        }
        index++;
    }
}


void use_redirection(bool *validExecution, char ***arguments, size_t symbolsFound, int *tokenIndex,
                     int indirect, int outdirect, int din, int dout) {
    for (int row = 0; row < symbolsFound + 1; row++) {
        for (int col = 0; col < *tokenIndex + symbolsFound; col++) {
            if (arguments[row][col] != NULL) {
                if (strcmp(arguments[row][col], "<") == 0) {
                    indirect = col + 1;
                    if (indirect) {
                        // your indirect is called. see end of this file
                        is_indirect(arguments, row, indirect, din, validExecution);
                    }
                } else if (strcmp(arguments[row][col], ">") == 0) {
                    outdirect = col + 1;
                    if (outdirect) {
                        // your outdirect is called. see end of this file
                        is_outdirect(arguments, row, outdirect, dout, validExecution);
                    }
                }
            }
        }
    }
}

void is_indirect(char ***arguments, int iRow, int indirect, int din, bool *validExecution) {
    //seek_wildcard(instructions);
    din = open(arguments[iRow][indirect], O_RDONLY);
    if (din == -1) {
        printf("Failed to open file! (%s)\n", strerror(errno));
        *validExecution = false;
        return;
    }
    dup2(din, STDIN_FILENO);
    close(din);
    arguments[iRow][indirect - 1] = NULL;
}

void is_outdirect(char ***arguments, int iRow, int outdirect, int dout, bool *validExecution) {
    //seek_wildcard(instructions);
    dout = open(arguments[iRow][outdirect], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
    if (dout == -1) {
        printf("Failed to to open file! (%s)\n", strerror(errno));
        *validExecution = false;
        return;
    }
    dup2(dout, STDOUT_FILENO);
    close(dout);
    arguments[iRow][outdirect - 1] = NULL;
}


