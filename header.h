#ifndef PROJECT_2_BUILTINFUNCTIONS_H
#define PROJECT_2_BUILTINFUNCTIONS_H
#endif //PROJECT_2_BUILTINFUNCTIONS_H

#include <sys/types.h>
#include <stdbool.h>

#define INTERACTIVE 1
#define BATCH 2

void builtin_cd(char *const *instructions);

void builtin_pwd();

void use_fork_basic(char *cmd, char **parameters, char *const *instructions, bool *validExecution);

void fork_adv(char **instructions, bool *validExecution, int *tokenIndex);

void
find_path(char **instr, bool *validExecution);

void
read_command(char *par[], char *instr[], char *fname, int type, int *tokenIndex, off_t *filePtrPos,
             const off_t *filePtrEndPos);

void helper_input(int fd, int type, char *fname, char *payload, off_t *filePtrPos);

void helper_create_tokens(char *par[], char *instr[], char *payload, char *tokens, int *tokenIndex);

char *curr_line();

void free_pointers(char *cmd, char **parameters, char **instructions, int *tokenIndex);

void prompt_interactive();

void prompt_interactive_normal();

void prompt_interactive_error();

void prompt_batch();

void prompt_goodbye();