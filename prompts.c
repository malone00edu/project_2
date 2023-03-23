#define _GNU_SOURCE

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "header.h"

void prompt_interactive() {
    static int initial = true;
    if (initial) {
        system("clear");
        char *readyBuf = {"---Welcome to my shell!---\n"};
        write(STDOUT_FILENO, readyBuf, strlen(readyBuf));
        initial = false;
    }


}

void prompt_interactive_normal() {
    char *writeBuf = {"mysh> "};
    write(STDOUT_FILENO, writeBuf, strlen(writeBuf));
}

void prompt_interactive_error() {
    char *writeBuf = {"!mysh> "};
    write(STDOUT_FILENO, writeBuf, strlen(writeBuf));
}

void prompt_batch() {
    static int initial = true;
    if (initial) {
        system("clear");
        char *readyBuf = {"---Welcome to my shell!---\n"};
        write(STDOUT_FILENO, readyBuf, strlen(readyBuf));
        initial = false;
    }
}

void prompt_goodbye() {
    char *byeBuf = {"---Exiting shell. Goodbye!---\n"};
    write(STDOUT_FILENO, byeBuf, strlen(byeBuf));

}