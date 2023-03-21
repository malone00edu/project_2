#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/wait.h>


void interactiveMode();
void batchMode(char* filename);



int main(int argc, char *argv[]) {
    if (argc == 2) {
      /*batch*/
    } 
    else {
        interactiveMode();
    }

    return 0;
}

void interactiveMode() {
    int stop = 0;

    printf("-- Interactive Mode -- \n");
    printf("Welcome to my shell! Input 'exit' to leave. \n ");

    char line[100];
    char cmd[10];

    while (stop == 0) {
        // clear the cmd array at the start of each loop
        memset(cmd, 0, sizeof(cmd));

        printf("mysh> ");
        fgets(line, sizeof(line), stdin);

        // Split the input into individual arguments
        char* token = strtok(line, " \t\n");
        if (token != NULL) {
            strncpy(cmd, token, sizeof(cmd) - 1);
            cmd[sizeof(cmd) - 1] = '\0';
        }

        // stop program
        if (strcmp(cmd, "exit") == 0) {
            stop = 1;
        }
        // cd command
        // also implemented home shortcut
        else if (strcmp(cmd, "cd") == 0) {
            char* dir = strtok(NULL, "\n");
            if (dir != NULL) {
                if (strncmp(dir, "~/", 2) == 0) {
                    // replace the tilde with the value of HOME
                    char* home = getenv("HOME");
                    if (home == NULL) {
                        printf("!mysh> HOME environment variable missing\n");
                    } else {
                        char newdir[strlen(home) + strlen(dir) - 1];
                        sprintf(newdir, "%s%s", home, dir + 1);
                        if (chdir(newdir) != 0) {
                            printf("!mysh> No such file or directory.\n");
                        }
                    }
                } else {
                    if (chdir(dir) != 0) {
                        printf("!mysh> No such file or directory.\n");
                    }
                }
            } else {
                char* home = getenv("HOME");
                if (home == NULL) {
                    printf("!mysh> HOME environment variable missing.\n");
                } else {
                    if (chdir(home) != 0) {
                        printf("!mysh> No such file or directory.\n");
                    }
                }
            }
        }
        // execute command

        else {
            pid_t pid = fork();
            if (pid == 0) {
                // child process
                char* args[100];
                int index = 0;
                args[index++] = cmd;
                char* token;
                while ((token = strtok(NULL, " \t\n")) != NULL) {
                    args[index++] = token;
                }
                args[index] = NULL;
                execvp(cmd, args);
                perror("!mysh> Execution failed");
                exit(1);
            } 
            else if (pid < 0) {
                // fork failed
                perror("!mysh> Fork failed");
                exit(1);
            } 
            else {
                //parent 
                wait(NULL);
            }
        }
    }
    printf("Exit successfully! \n");
}


