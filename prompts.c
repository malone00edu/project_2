#define _GNU_SOURCE

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

void prompt_homeerr(){
    char *enverr = {"!mysh> HOME environment variable missing"};
    write (STDOUT_FILENO, enverr, strlen(enverr));
}

void prompt_nodir(){
    char *nodir= {"!mysh> No such file or directory.\n"};
    write (STDOUT_FILENO, nodir, strlen(nodir)); 
}
