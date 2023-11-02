#include "../include/shell.h"
#include "command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>

List childProcsList;

int histoty_recent_index = -1;
char *history[HISTORY_SIZE];
char *input_buffer[1];  // insert by replay

// loacl variable
bool isBackground = false;

void ShellInit() {
    isBackground;
    // init child process recorder
    childProcsList.sz = 0;
    childProcsList.head = NULL;
    // init input buffer
    input_buffer[0] = NULL;
    // init history
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history[i] = NULL;
    }
    // start shell
    hello();
}


void ShellLoop() {
    char *line, *origin_line;
    char **instructions;
    int status, num_instruction;

    do {
        printf(">>> $");
        line = ShellRead();
        origin_line = strdup(line);
        num_instruction = ShellInstructParse(line, &instructions);
        if ((num_instruction == 1 || num_instruction == 0) &&
            (instructions[0] == NULL || strcmp(instructions[0], "") == 0 || strcmp(instructions[0], " ") == 0 ||
             strcmp(instructions[0], "\t") == 0)) {
            continue;
        }

        status = ShellExecute(0, instructions, num_instruction, origin_line);

        free(line);
        free(origin_line);
        free(instructions);
    } while (status);
}

char *ShellRead() {
    char *buffer = NULL;
    int bufflen = 0;
    int strLen = 0;
    if (input_buffer[0] == NULL) {
        if ((strLen = getline(&buffer, &bufflen, stdin)) == -1) {
            if (feof(stdin)) {
                exit(EXIT_SUCCESS);
            } else {
                perror("ShellRead: getline Error");
                exit(EXIT_FAILURE);
            }
        }
        if (buffer[strLen - 1] == '\n')
            buffer[strLen - 1] = '\0';
    } else {
        printf("\b\b\b\b\b\b");
        buffer = input_buffer[0];
        input_buffer[0] = NULL;
    }
    return buffer;
}

int ShellInstructParse(char *line, char ***instructions) {
    int cnt = split_line(line, "|", instructions);

    isBackground = false;
    if (cnt > 0) {
        int last_str_len = strlen((*instructions)[cnt - 1]);
        if ((*instructions)[cnt - 1][last_str_len - 1] == '&') {
            isBackground = true;
            (*instructions)[cnt - 1][last_str_len - 2] = '\0';
        }
    }
    (*instructions)[cnt] = NULL;
    return cnt;
}

char **ShellParse(char *line) {
    char **tokens;
    int cnt = split_line(line, TOK_DELIM, &tokens);

    tokens[cnt] = NULL;
    return tokens;
}

int ShellLaunch(char **tokens) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // child process
        if (execvp(tokens[0], tokens) == -1) {
            perror("ShellLaunch: child execlp");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("ShellLaunch: fork failed");
    } else {
        // parent process
        if (!isBackground) {
            do {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        } else {
            printf("[Pid]: %d\n", pid);
        }
    }
    return 1; // we can allow the input again
}

int ShellExecute(int i_pipe, char **instructions, int num_instruction, char *origin_line) {
    char **tokens = ShellParse(instructions[i_pipe]);

    PipeHandler(i_pipe, instructions, num_instruction, origin_line);
    if (strcmp(tokens[0], "replay") != 0) {
        HistoryInsert(origin_line);
    }
    for (int i = 0; i < NumBuiltins(); i++) {
        if (strcmp(tokens[0], builtin_strCmd[i]) == 0) {
            return (*builtin_func[i])(tokens);
        }
    }
    return ShellLaunch(tokens);
}

void HistoryClear() {
    for (int i = 1, curr = histoty_recent_index; i <= HISTORY_SIZE && history[curr] != NULL; i++) {
        free(history[curr]);
        curr = (curr - 1 + HISTORY_SIZE) % HISTORY_SIZE;
    }
}

void HistoryInsert(char *line) {
    if (line == NULL)
        return;

    int curr = (histoty_recent_index + 1) % HISTORY_SIZE;
    if (history[curr] != NULL) {
        free(history[curr]);
    }
    history[curr] = strdup(line);
    histoty_recent_index = curr;
}

void InputBufferInsert(char *line) {
    input_buffer[0] = strdup(line);
}

Node *CreateNewNode(pid_t pid) {
    Node *new_node = malloc(sizeof(*new_node));
    new_node->next = NULL;
    new_node->pid = pid;
    return new_node;
}

void ChildProcsInsert(pid_t cpid) {
    Node *new_node = CreateNewNode(cpid);
    if (childProcsList.sz == 0) {
        childProcsList.head = new_node;
    } else {
        new_node->next = childProcsList.head->next;
        childProcsList.head = new_node;
    }
    childProcsList.sz++;
}

void ChildProcsRemove(Node *node) {
    Node **curr = &(childProcsList.head);
    if (childProcsList.sz != 0) {
        while ((*curr) != node) {
            curr = &((*curr)->next);
        }
        (*curr) = node->next;
        free(node);
        childProcsList.sz--;
    }
}

void PipeHandler(int i_pipe, char **instructions, int num_instruction, char *origin_line) {
    if (instructions[i_pipe + 1] != NULL) {
        int pipeFd[2];
        if (pipe(pipeFd) < 0)
            perror("PipeHandler: pipe create error");
        pid_t cpid = fork();
        if (cpid == 0) {
            // child process
            close(pipeFd[1]);
            dup2(pipeFd[0], STDIN_FILENO);
            close(pipeFd[0]);

            ShellExecute(i_pipe + 1, instructions, num_instruction, origin_line);
        } else if (cpid < 0) {
            perror("ShellLaunch: fork failed");
        } else {
            // parent process
            close(pipeFd[0]);
            dup2(pipeFd[1], STDOUT_FILENO);
            close(pipeFd[1]);

//            if(i_pipe == 0){
//                dup2()
//            }
        }
    }
}


