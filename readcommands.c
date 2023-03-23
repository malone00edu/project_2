#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>
#include "header.h"

static char *payload = NULL;
void read_command(char *par[], char *instr[], char *fname, int type, int *tokenIndex,
                  off_t *filePtrPos, off_t *filePtrEndPos) {
    int fd = STDIN_FILENO;; // if batch is not inuse. fd = 0 (STD_FILENO) will be used from henceforth.

    size_t size; // if a file isn't being used. a placeholder of 5000 bytes will be used for read() buffer.
    char *tokens = NULL;
    static int initialCall = true;
    if (initialCall) { // creates payload to store instructions.
        if (type == BATCH) { // find the end position of given file and allocate memory based on returned value (size).
            fd = open(fname, O_RDONLY);
            if (fd < 0) {
                perror("Error.");
                exit(1);
            }
            size = lseek(fd, 0, SEEK_END);
            close(fd);
            payload = calloc(size + 1, sizeof(char));

        }
        if (type == INTERACTIVE) {
            size = 5000;
            payload = calloc(size, sizeof(char));
        }
        initialCall = false;
    }

    helper_input(fd, type, fname, filePtrPos);
    helper_create_tokens(par, instr, tokens, tokenIndex);
    if (instr[0] != NULL) {
        if (((strcmp(instr[0], "exit")) == 0) | (*filePtrEndPos == *filePtrPos)) {
            free(payload);
        }
    }
}

void helper_input(int fd, int type, char *fname, off_t *filePtrPos) {
    ssize_t bytesRead = 0;
    ssize_t totalBytesRead = 0;
    char line[5000];
    memset(line, 0, 5000 * sizeof(char)); // clear array of garbage data.

    if (type == BATCH) { // if file exists, will continue to read where pointer (filePointerPos) previously left off.
        fd = open(fname, O_RDONLY);
        if (fd < 0) {
            perror("Error. ");
            exit(1);
        }
        lseek(fd, *filePtrPos, SEEK_SET);
    }
    /* read 1 byte at a time and store in line[]. line + totalBytesRead is needed so read() is not constantly rewriting
    in the 1st char position of line (i.e line[0]). */
    while ((bytesRead = read(fd, line + totalBytesRead, sizeof(char))) >= 0) {
        totalBytesRead += bytesRead; //tally total number of bytes read before the if statement below forces a break.
        if ((strcmp(&line[totalBytesRead - 1], "\n") == 0) | (bytesRead == 0)) { // use current line (byte data).
            strcpy(payload, line);
            if (type == BATCH) {
                *filePtrPos = lseek(fd, 0, SEEK_CUR); // save the last position of file ptr
                close(fd);
            }
            break;
        }
    }
}

void helper_create_tokens(char *par[], char *instr[], char *tokens,
                          int *tokenIndex) { // create tokens from the payload
    *tokenIndex = 0;

    tokens = strtok(payload, " \n"); // create first token.

    while (tokens != NULL) {
        instr[(*tokenIndex)++] = strdup(tokens); // insert token into instructions array
        tokens = strtok(NULL, " \n"); // get/create next token.
    }


    for (int j = 0; j < *tokenIndex; j++) { // create parameters from instructions. needed for execv().
        par[j] = instr[j];
    }
    par[*tokenIndex] = NULL; // insert NULL into this position. needed for execv().
}

char *curr_payload(){
    return payload;
}