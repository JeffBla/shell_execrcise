#ifndef BULITIN_H
#define BULITIN_H

#include "../include/shell.h"

void hello();

int ShellHelp(char **args);

int ShellCd(char **args);

int ShellEcho(char **args);

int ShellRecord(char **args);

int ShellReplay(char **args);

int ShellMypid(char **args);

int ShellExit(char **args);

extern char *builtin_strCmd[];

extern int (*builtin_func[])(char **);

int NumBuiltins();

#endif