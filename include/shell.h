#ifndef SHELL_H
#define SHELL_H

#include "../include/builtin.h"

#include <unistd.h>

#define BUFFER_SIZE 1000
#define HISTORY_SIZE 16
#define TOK_DELIM " \t\r\n\a"

typedef struct Node Node;
typedef struct List List;

struct Node {
    pid_t pid;
    Node *next;
};

struct List {
    Node *head;
    size_t sz;
};

extern List childProcsList;

extern int histoty_recent_index;
extern char *history[HISTORY_SIZE];
extern char *input_buffer[1];  // insert by replay

void ShellInit();

void ShellLoop();

char *ShellRead();

int ShellInstructParse(char *line, char ***instructions);

char **ShellParse(char *line);

int ShellLaunch(char **tokens);

int ShellExecute(int i_pipe, char **instructions, int num_instruction, char *origin_line);

void HistoryClear();

void HistoryInsert(char *line);

void InputBufferInsert(char *line);

void ChildProcsInsert(pid_t cpid);

void ChildProcsRemove(Node *node);

void PipeHandler(int i_pipe, char **instructions, int num_instruction, char *origin_line);

#endif