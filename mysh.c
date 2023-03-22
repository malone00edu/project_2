#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include "header.h"

int main(int argc, char *argv[]) {
    int *tokenIndex = calloc(1, sizeof(int)); // total tokens created for the current set of instructions. needed for free();
    int *filePtrPos = calloc(1, sizeof(int)); // current position of file with respect to read().
    int *filePtrEndPos = calloc(1, sizeof(int)); // gives the end position of file with the use of lseek()

    *tokenIndex = 0;
    *filePtrPos = 0;
    *filePtrEndPos = -1; // -1 is a placeholder.

    int type = 0;
    char *filename = NULL;

    if (argc == 1) { // no file given
        type = INTERACTIVE;
    }
    if (argc == 2) { // a file is given to use
        type = BATCH;
        filename = strdup(argv[1]);
        int fd = open(filename, O_RDONLY);
        if (fd < 0) {
            perror("Error. ");
            exit(1);
        }
        *filePtrEndPos = lseek(fd, 0, SEEK_END); // find total size of given file.
        close(fd);
    }

    char *cmd; // stores the complete path to execute.
    char **parameters; // needed for execv(). see below.
    char **instructions; // the current set of instructions will be held as a series of pointers (AKA our tokens).

    while (true) { // repeat forever until user enters "exit" or no instructions are given.
        cmd = calloc(5000, sizeof(char));
        instructions = malloc(50 * sizeof(char *));
        parameters = malloc(50 * sizeof(char *));

        for(int i = 0; i < 50; i++){
            instructions[i] = NULL;
            parameters[i] = NULL;
        }

        if (type == INTERACTIVE) { // if no file is given. display greeting and "mysh>". else display greeting.
            prompt_interactive();
        } else {
            prompt_batch();
        }

        read_command(parameters, instructions, filename, type, tokenIndex, filePtrPos, filePtrEndPos);

        // exit while loop under these conditions: "exit" in file | typed by user | bytes read from a file = 0 (EOF).
        if (instructions[0] != NULL) {
            if (((strcmp(instructions[0], "exit")) == 0) | (*filePtrEndPos == *filePtrPos)) {
                prompt_goodbye();
                free_pointers(cmd, parameters, instructions, tokenIndex);
                break;
            }
        }

        // This if else is a proof of concept. It will probably look completely different after project requirements.
        if (instructions[0] != NULL) {
            if (strcmp(instructions[0], "cd") == 0) {
                builtin_cd(instructions);
            }// add an else if statement here for builtin pwd command
            else if (strcmp(instructions[0], "pwd") == 0) {
                builtin_pwd();
            }
            else {
                if (fork() != 0) { // parent
                    wait(NULL); // wait for child
                } else {
                    strcpy(cmd, "/bin/"); // this is a placeholder. just a proof of concept to see if this thing works.
                    strcat(cmd, instructions[0]); // instructions[0] will always hold 1st command, ls, pwd, where, cat, etc..
                    execv(cmd, parameters);  // execute command

                }
            }
        }
        // free allocated memory and get the next set of instructions by repeating this while loop.
        free_pointers(cmd, parameters, instructions, tokenIndex);
    }
    free(tokenIndex);
    free(filePtrPos);
    free(filePtrEndPos);
    exit(0);
}


