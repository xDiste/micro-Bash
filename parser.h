#ifndef _PARSER_H
#define _PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAXLEN 1024 // constant that defines the maximum generic length for arrays

bool checkPipe(char *line);
void parser_seq(char *line);
void parser_cmd(char *line, char **buff);
void runCommand(char **buff);
void runCd(char **cmd_cd, char *curr_path);

#endif