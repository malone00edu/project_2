#ifndef PROJECT_2_BUILTINFUNCTIONS_H
#define PROJECT_2_BUILTINFUNCTIONS_H
#endif //PROJECT_2_BUILTINFUNCTIONS_H

#define INTERACTIVE 1
#define BATCH 2

void builtin_cd(char *const *instructions);
void builtin_pwd();
void
read_command(char *par[], char *instr[], char *fname, int type, int *tokenIndex, int *filePtrPos, int *filePtrEndPos);
void helper_input(int fd, char *payload, int type, char *fname, int *filePtrPos);
void helper_create_tokens(char *par[], char *instr[], char *payload, char *tokens, int *tokenIndex);
void free_pointers(char *cmd, char **parameters, char **instructions, int *tokenIndex);
void prompt_interactive();
void prompt_batch();
void prompt_goodbye();