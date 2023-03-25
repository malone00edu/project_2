                /*original else*/

            else {

                if (fork() != 0) { // parent
                    wait(NULL); // wait for child
                } else {
                    strcpy(cmd, "/bin/"); // this is a placeholder. just a proof of concept to see if this thing works.
                    strcat(cmd, instructions[0]); // instructions[0] will always hold 1st command, ls, pwd, where, cat, etc..
                    execv(cmd, parameters);  // execute command

                }

            }

/*redirection code*/

        else {
            
            
            int pid = fork();
            if (pid == 0) { // child process
            // check if input or output redirection is needed
            int indirect = 0;
            int outdirect = 0;
            int din=0;
            int dout;
            for (int i = 0; parameters[i] != NULL; i++) {
                if (strcmp(parameters[i], "<") == 0) {
                    indirect = i + 1;
                } else if (strcmp(parameters[i], ">") == 0) {
                    outdirect = i + 1;
                }
            }

    // execute command with input/output redirection
            if (indirect) {
                din = open(parameters[indirect], O_RDONLY);
                if (din == -1) {
                    perror("open");
                    exit(1);
                
                    dup2(din, STDIN_FILENO);
                    close(din);
                    parameters[indirect-1] = NULL;
                }
                if (outdirect) {
                    dout = open(parameters[outdirect], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
                    if (dout == -1) {
                        perror("open");
                        exit(1);
                    }
                        dup2(dout, STDOUT_FILENO);
                        close(dout);
                        parameters[outdirect-1] = NULL;
                }

                strcpy(cmd, "/bin/");
                strcat(cmd, instructions[0]);
                execv(cmd, parameters);
                perror("execv"); // should never reach here
                exit(1);
                } 
                else if (pid > 0) { // parent process
                    wait(NULL);
                } else { // fork() failed
                    perror("fork");
                    exit(1);
                }

            }
        }

/* wildcard and redirection - still a lot of errors */
else {
    /* check for redirections */
    int id = fork();
    if (id == 0) {
        int din;
        int dout;
        int indirect = 0;
        int outdirect = 0;
     
        for (int i = 0; parameters[i]!= NULL; i++) {
            if (strcmp(parameters[i], "<") == 0) {
                indirect = i + 1;
          
            } else if (strcmp(parameters[i], ">") == 0) {
                outdirect = i + 1;
          
            }
        }

        /* check for wildcards */
        int track = 0;
        for (int i = 0; instructions[i] !=NULL; i++) {
            if (strchr(instructions[i], '*') != NULL) {
                glob_t paths;
                glob(instructions[i], GLOB_NOCHECK | GLOB_TILDE, NULL, &paths);
                track += paths.gl_pathc;
                globfree(&paths);
            } else {
                track++;
            }
        }

        int argi = 0;

        char **argms = malloc((track + 1) * sizeof(char *));
        

        for (int i = 0; instructions[i] != NULL; i++) {
            if (strcmp(instructions[i],"<") == 0 || strcmp(instructions[i], ">") == 0) {
                continue;
            }
            if (strchr(instructions[i], '*') != NULL) {
                glob_t paths;
                glob(instructions[i], GLOB_NOCHECK | GLOB_TILDE, NULL, &paths);
                for (int j = 0; j < paths.gl_pathc; j++) {
                    argms[argi++] = strdup(paths.gl_pathv[j]);
                }
                globfree(&paths);
            } else {
                argms[argi++] = strdup(instructions[i]);
            }
        }

        argms[argi] = NULL;

        if (fork() == 0) { // child process
            // execute command with input/output redirection
            if (indirect) {
                din = open(parameters[indirect], O_RDONLY);
                if (din == -1) {
                    perror("open");
                    exit(1);
                }
                dup2(din, STDIN_FILENO);
                close(din);
            }
            if (outdirect) {
                dout = open(parameters[outdirect], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
                if (dout == -1) {
                    perror("open");
                    exit(1);
                }
                dup2(dout, STDOUT_FILENO);
                close(dout);
            }

            strcpy(cmd, "/bin/");
            strcat(cmd, argms[0]);
            execv(cmd, argms);
            perror("execv"); 
            exit(1);
        }

        for (int i = 0; i < argi; i++) {
            free(argms[i]);
        }

        free(argms);

        exit(0);
    } // parent process
    else if (id > 0) { 
        wait(NULL);
    } else { // fork() failed
        perror("fork");
        exit(1);
    }
}
