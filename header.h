#ifndef PROJECT_2_BUILTINFUNCTIONS_H
#define PROJECT_2_BUILTINFUNCTIONS_H
#endif //PROJECT_2_BUILTINFUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <glob.h>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#define INTERACTIVE 1
#define BATCH 2

void builtin_cd(char *const *instructions);

void builtin_pwd();

void is_indirect(char ***arguments, int row, int indirect, int din, bool *validExecution);

void is_outdirect(char ***arguments, int row, int outdirect, int dout, bool *validExecution);

void
use_redirection(bool *validExecution, char ***arguments, const size_t *arrRows, const size_t *arrCols, int indirect, int outdirect,
                int din, int dout);

void create_2d_array(char *const *instructions, const int *tokenIndex, bool *validExecution, size_t symbolsFound,
                     size_t *arrRows, size_t *arrCols, char ****arguments, char **cmd1, char **cmd2);

void check_for_wildcard(char *const *instructions, char ****arguments, int index, int rowPos, int colPos);

void execute(char **instructions, int *tokenIndex, bool *validExecution);

void
read_command(char *par[], char *instr[], char *fname, int type, int *tokenIndex, off_t *filePtrPos,
             const off_t *filePtrEndPos);

void find_path(char **cmd, bool *validExecution);

void helper_input(int fd, int type, char *fname, char *payload, off_t *filePtrPos);

void helper_create_tokens(char *par[], char *instr[], char *payload, char *tokens, int *tokenIndex);

void free_pointers(char *cmd, char **parameters, char **instructions, int *tokenIndex);

void prompt_interactive();

void prompt_interactive_normal();

void prompt_interactive_error();

void prompt_batch();

void prompt_goodbye();

void prompt_homeerr();

void prompt_nodir();




