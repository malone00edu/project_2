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
#include <dirent.h>

#include <glob.h>

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
        instructions = calloc(50, sizeof(char *));
        parameters = malloc(50 * sizeof(char *));

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
                
                /*
                char **args;
                int arg_index=0;
                expand_instr(instructions, &args, arg_index); */
                
                //checks for wildcards - expand the arguments using glob
                int count = 0;
                for (int i = 0; instructions[i] != NULL; i++) {
                    if (strchr(instructions[i], '*') != NULL) {
                        glob_t paths;
                        glob(instructions[i], GLOB_NOCHECK | GLOB_TILDE, NULL, &paths);
                        count += paths.gl_pathc;
                        globfree(&paths);
                    } 
                    else {
                        count++;
                    }
                }           

                char **args = malloc((count + 1) * sizeof(char *));
                int arg_index = 0;

                for (int i = 0; instructions[i] != NULL; i++) {
                    if (strchr(instructions[i], '*') != NULL) {
                        glob_t paths;
                        glob(instructions[i], GLOB_NOCHECK | GLOB_TILDE, NULL, &paths);
                        for (int j = 0; j < paths.gl_pathc; j++) {
                            args[arg_index++] = strdup(paths.gl_pathv[j]);
                        }
                        globfree(&paths);
                    } 
                    else {
                        args[arg_index++] = strdup(instructions[i]);
                    }
                }

                args[arg_index] = NULL;
                

                if (fork() != 0) { // parent
                    wait(NULL); // wait for child
                } else {
                    strcpy(cmd, "/bin/"); // this is a placeholder. just a proof of concept to see if this thing works.
                    strcat(cmd, args[0]); // args[0] will always hold 1st command, ls, pwd, where, cat, etc..
                    execv(cmd, args);  // execute command
                }

                for (int i = 0; i < arg_index; i++) {
                    free(args[i]);
                }
                
                free(args);
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


