#include "../include/shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>

int histoty_recent_index = -1;
char *history[HISTORY_SIZE];
char *input_buffer[1];  // insert by replay
List childProcsList;

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
    char **tokens;
    int status;

    do {
        printf(">>> $");
        line = ShellRead();
        origin_line = strdup(line);
        tokens = ShellParse(line);
        status = ShellExecute(tokens, origin_line);

        free(line);
        free(origin_line);
        free(tokens);
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

char **ShellParse(char *line) {
    int bufsize = BUFFER_SIZE, cnt = 0;
    char **tokens = malloc(BUFFER_SIZE * sizeof(tokens[0]));
    if (!tokens) {
        fprintf(stderr, "ShellParse: allocation error\n");
        exit(EXIT_FAILURE);
    }

    char *token;
    char *rest_of_str = NULL;
    token = strtok_r(line, TOK_DELIM, &rest_of_str);
    while (token != NULL) {
        tokens[cnt++] = token;
        if (cnt >= bufsize) {
            bufsize += BUFFER_SIZE;
            tokens = realloc(tokens, bufsize * sizeof(tokens[0]));
            if (!tokens) {
                fprintf(stderr, "ShellParse: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok_r(rest_of_str, TOK_DELIM, &rest_of_str);
    }
    if (cnt > 0 && tokens[cnt - 1][0] == '&') {
        isBackground = true;
        tokens[cnt - 1] = NULL;
    } else {
        isBackground = false;
        tokens[cnt] = NULL;
    }
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
//            ChildProcsInsert(pid);
        }
    }
    return 1; // we can allow the input again
}

int ShellExecute(char **tokens, char *origin_line) {
    if (tokens[0] == NULL || strcmp(tokens[0], "") == 0 || strcmp(tokens[0], " ") == 0 ||
        strcmp(tokens[0], "\t") == 0) {
        return 1;
    }

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




