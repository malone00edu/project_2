#define _GNU_SOURCE

void chk_for_wildcards(const int *tokenIndex, char *cmd, char *const *instructions);
void chk_for_redirect(const int *tokenIndex, char *cmd, char *const *instructions, char **parameters);

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <glob.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include "header.h"
#include <dirent.h>



int main(int argc, char *argv[]) {

    bool *validExecution = calloc(1, sizeof(bool));
    *validExecution = true;
    int *tokenIndex = calloc(1, sizeof(int)); // total tokens created for the curr set of instr. needed for free();
    off_t *filePtrPos = calloc(1, sizeof(off_t)); // current position of file with respect to read().
    off_t *filePtrEndPos = calloc(1, sizeof(off_t)); // gives the end position of file with the use of lseek()


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

        for (int i = 0; i < 50; i++) {
            instructions[i] = NULL;
            parameters[i] = NULL;
        }

        if (type == INTERACTIVE) { // if no file is given. display greeting and "mysh>". else display greeting.
            prompt_interactive();
            if (*validExecution) {
                prompt_interactive_normal();
            } else {
                prompt_interactive_error();
                *validExecution = true;
            }

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
            if (strcmp(instructions[0], "cd") == 0) {
                builtin_cd(instructions);
            } else if (strcmp(instructions[0], "pwd") == 0) {
                builtin_pwd();
            } else if (strchr(instructions[0], '/') != NULL) {
                use_fork_basic(cmd, parameters, instructions, validExecution);
            } else {
               // fork_adv(instructions, validExecution, tokenIndex); // else no path given.
                chk_for_redirect(tokenIndex,cmd, instructions,parameters);
                /*char **args; int arg_index=0; expand_instr(instructions, &args, arg_index); */
                
                //checks for wildcards - expand the arguments using glob
                //chk_for_wildcards(tokenIndex, cmd, instructions);


            }
        }
        // free allocated memory and get the next set of instructions by repeating this while loop.
        free_pointers(cmd, parameters, instructions, tokenIndex);
    }
    free(validExecution);
    free(tokenIndex);
    free(filePtrPos);
    free(filePtrEndPos);
    exit(0);
}

void chk_for_wildcards(const int *tokenIndex, char *cmd, char *const *instructions) {
    int count = 0;
    for (int i = 0; i < *tokenIndex; i++) { // Replaced instructions[i] != NULL w/ *tokenIndex. See line 17
        if (strchr(instructions[i], '*') != NULL) {
            glob_t paths;
            glob(instructions[i], GLOB_NOCHECK | GLOB_TILDE, NULL, &paths);
            count += paths.gl_pathc;
            globfree(&paths);
        } else {
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
        } else {
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

//*checks for redirects OR pipes NOT BOTH!
void chk_for_redirect(const int *tokenIndex, char *cmd, char *const *instructions, char **parameters){
    int pid = fork();
    if (pid == 0) { // child process
        // check if input or output redirection is needed
        int indirect = 0;
        int outdirect = 0;
        int din = 0;
        int dout = 0;
        int pipefd[2] = {-1, -1}; // initialize pipe file descriptors
        int pipeIndex = 0; // initialize pipe index
        for (int i = 0; parameters[i] != NULL; i++) {
            if (strcmp(parameters[i], "<") == 0) {
                indirect = i + 1;
            } else if (strcmp(parameters[i], ">") == 0) {
                outdirect = i + 1;
            } else if (strcmp(parameters[i], "|") == 0) {
                pipeIndex = i;
            }
        }

        // execute command with input/output redirection and/or pipes
        if (pipeIndex) {
            // create pipe
            if (pipe(pipefd) == -1) {
                perror("pipe");
                exit(1);
            }

            // fork another process to execute the command after the pipe
            int pid2 = fork();
            if (pid2 == 0) {
                // close read end of the pipe
                close(pipefd[1]);

                // set the input file descriptor to the read end of the pipe
                dup2(pipefd[0], STDIN_FILENO);

                // execute the command after the pipe
                strcpy(cmd, "/bin/");
                strcat(cmd, &parameters[pipeIndex+1][0]);
                execv(cmd, &parameters[pipeIndex+1]);

 
                perror("execv");
                exit(1);
            } else if (pid2 > 0) {
                // close write end of the pipe
                close(pipefd[0]);

                // set the output file descriptor to the write end of the pipe
                dup2(pipefd[1], STDOUT_FILENO);

                // execute the command before the pipe
                parameters[pipeIndex] = NULL;
                strcpy(cmd, "/bin/");
                strcat(cmd, &parameters[0][0]);
                execv(cmd, parameters);

         
                perror("execv");
                exit(1);
            } else {
                // fork() failed
                perror("fork failed");
                exit(1);
            }
        } else {
            if (indirect) {
                din = open(parameters[indirect], O_RDONLY);
                if (din == -1) {
                    perror("open");
                    exit(1);
                }
                dup2(din, STDIN_FILENO);
                close(din);
                parameters[indirect - 1] = NULL;
            }
            if (outdirect) {
                dout = open(parameters[outdirect], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
                if (dout == -1) {
                    perror("open");
                    exit(1);
                }
                dup2(dout, STDOUT_FILENO);
                close(dout);
                parameters[outdirect - 1] = NULL;
            }
            strcpy(cmd, "/bin/");
            strcat(cmd, instructions[0]);
            execv(cmd, parameters);
            perror("execv"); // should never reach here
            exit(1);
        }
    } else if (pid > 0) { // parent process
        wait(NULL);
    } else { // fork() failed
        perror("fork failed");
        exit(1);
    }

}

















