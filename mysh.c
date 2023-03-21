#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/fcntl.h>

#define INTERACTIVE 1
#define BATCH 2

static int  tokenIndex = 0; // total tokens created for the current set of instructions. needed for free();
static off_t filePtrPos = 0; // current position of file with respect to read()
static int bufSize = 10; // not in use.

char *increase_buffer_size(char *payload); // not in use.
void helper_create_tokens(char *par[], char *instr[], char *payload, char *tokens);
void helper_input(int fd, char *payload, int type, char *fname);

char *increase_buffer_size(char *payload) { // Not inuse yet... or ever?
    int multiplier = 100;
    bufSize = multiplier * bufSize;
    char *newSize = realloc(payload, bufSize * sizeof(char));
    if (newSize != NULL) {
        printf("Buffer size successfully increased!\n");
        payload = newSize;
        return payload;
    } else {
        printf("Couldn't resize array with realloc.\n");
        return payload; // placeholder.
    }
}

void type_prompt_interactive() {
    static int initial = true;
    if (initial) {
        system("clear");
        char *readyBuf = {"---Welcome to my shell!---\n"};
        write(STDOUT_FILENO, readyBuf, strlen(readyBuf));
        initial = false;
    }
    char *writeBuf = {"mysh>"};
    write(STDOUT_FILENO, writeBuf, strlen(writeBuf));
}

void type_prompt_batch() {
    static int initial = true;
    if (initial) {
        system("clear");
        char *readyBuf = {"---Welcome to my shell!---\n"};
        write(STDOUT_FILENO, readyBuf, strlen(readyBuf));
        initial = false;
    }
}

void type_goodbye() {
    char *byeBuf = {"---Exiting shell. Goodbye!---\n"};
    write(STDOUT_FILENO, byeBuf, strlen(byeBuf));

}

void read_command(char *par[], char *instr[], int type, char *fname) {
    int fd = 0; // if batch is not inuse. fd = 0 (STD_FILENO) will be used from henceforth.
    static char *payload = NULL;
    size_t size; // if a file isn't being used. a placeholder of 5000 bytes will be used for read() buffer.
    char *tokens = NULL;

    static int initialCall = true;
    if(initialCall) { // creates payload to store instructions.
        if (type == BATCH) { // find the end position of given file and allocate memory based on returned value (size).
            fd = open(fname, O_RDONLY);
            if(fd < 0) {
                perror("Error.");
                return;
            }
            size = lseek(fd, 0, SEEK_END);
            close(fd);
            payload = calloc(size + 1, sizeof(char));

        }
        if (type == INTERACTIVE) {
            fd = STDIN_FILENO; // standard helper_input for user commands.
            size = 5000;
            payload = calloc(size, sizeof(char));
        }
        initialCall = false;
    }

    helper_input(fd, payload, type, fname);
    helper_create_tokens(par, instr, payload, tokens);

    if (((strcmp(instr[0], "exit")) == 0) | ((strcmp(instr[0], "")) == 0)){
        free(payload);
    }
}

void helper_input(int fd, char *payload, int type, char *fname) {
    ssize_t bytesRead = 0;
    ssize_t totalBytesRead = 0;
    char line[5000];
    memset(line, 0, 5000 * sizeof(char)); // clear array of garbage data.

    if(type == BATCH) { // if file exists, will continue to read where pointer (filePointerPos) previously left off.
        fd = open(fname, O_RDONLY);
        lseek(fd, filePtrPos, SEEK_SET);
        if(fd < 0) {
            perror("Error");
            return;
        }
    }
    /* read 1 byte at a time and store in line[]. line + totalBytesRead is needed so read() is not constantly rewriting
    in the 1st char position of line (i.e line[0]). */
    while ((bytesRead = read(fd, line + totalBytesRead, sizeof(char))) >= 0) {
        totalBytesRead += bytesRead; //tally total number of bytes read before the if statement below forces a break.
        if ((strcmp(&line[totalBytesRead - 1], "\n") == 0)  | (bytesRead == 0)) { // examine current byte data.
            strcpy(payload, line);
            if(type == BATCH) {
                filePtrPos = lseek(fd, 0, SEEK_CUR); // save the last position of file ptr
                close(fd);
            }
            break;
        }
    }
}

void helper_create_tokens(char *par[], char *instr[], char *payload, char *tokens) { // create tokens from the line of instr
    tokenIndex = 0;
    tokens = strtok(payload, " \n"); // create first token.

    while (tokens != NULL) {
        instr[tokenIndex++] = strdup(tokens); // insert token into instructions array
        tokens = strtok(NULL, " \n"); // get next token.
    }

    for (int j = 0; j < tokenIndex; j++) { // create parameters from instructions. needed for execv().
        par[j] = instr[j];
    }
    par[tokenIndex] = NULL; // insert NULL into this position. needed for execv().
}

void free_pointers(char *cmd, char **parameters, char **instructions) { // free ptrs after ea. instruction | exit | EOF

    for (int x = 0; x < tokenIndex; x++) {
        free(instructions[x]);
    }
    free(instructions);
    free(parameters);
    free(cmd);
}


int main(int argc, char *argv[]) {
    int type = 0;
    char *filename = NULL;

    if(argc == 1){ // no file given
        type = INTERACTIVE;
    }
    if (argc == 2){ // a file is given to use
        type = BATCH;
        filename = strdup(argv[1]);
    }

    char *cmd; // stores the complete path to execute.
    char **parameters; // needed for execv(). see below.
    char **instructions; // the current set of instructions will be held as a series of pointers (AKA our tokens).
    // char* arr[] = {"ls", "-l", NULL};


    while (true) { // repeat forever until user enters "exit" or no instructions are given.
        cmd = malloc(5000 * sizeof(char));
        instructions = malloc(50 * sizeof(char *));
        parameters = malloc(50 * sizeof(char *));

        if(type == INTERACTIVE) { // if no file is given. display greeting and "mysh>". else display greeting.
            type_prompt_interactive(); 
        }
        else{
            type_prompt_batch(); 
        }
        
        read_command(parameters, instructions, type, filename); // read from a source & create tokens.

        // exit while loop under these conditions: "exit" in file | typed by user | bytes read from a file = 0 (EOF).
        if (((strcmp(instructions[0], "exit")) == 0) | ((strcmp(instructions[0], "")) == 0)) {
            free_pointers(cmd, parameters, instructions);
            break;
        }

        // This if else is a proof of concept. It will probably look completely different after project requirements.
        if (fork() != 0) { // parent
            wait(NULL); // wait for child
        } else {
            strcpy(cmd, "/bin/"); // this is a placeholder. just a proof of concept to see if this concept thing works.
            strcat(cmd, instructions[0]); // instructions[0] will always hold 1st command, ls, pwd, where, cat, etc..
            execv(cmd, parameters);  // execute command
        }
        // free allocated memory and get the next set of instructions by repeating this while loop.
        free_pointers(cmd, parameters, instructions);
    }
    type_goodbye();
    exit(0);
}
