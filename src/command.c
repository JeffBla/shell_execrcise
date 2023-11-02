#include "../include/shell.h"
#include "../include/command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int split_line(char *line, const char *delim, char ***tokens) {
    int bufsize = BUFFER_SIZE, cnt = 0;
    char **pre_tokens;
    *tokens = malloc(BUFFER_SIZE * sizeof(char *));
    if (!(*tokens)) {
        fprintf(stderr, "split_line: allocation error\n");
        exit(EXIT_FAILURE);
    }

    char *token;
    char *rest_of_str = NULL;
    token = strtok_r(line, delim, &rest_of_str);
    while (token != NULL) {
        (*tokens)[cnt++] = token;
        if (cnt >= bufsize) {
            bufsize += BUFFER_SIZE;
            pre_tokens = *tokens;
            *tokens = realloc(*tokens, bufsize * sizeof(tokens[0]));
            if (!(*tokens)) {
                fprintf(stderr, "split_line: allocation error\n");
                free(pre_tokens);
                exit(EXIT_FAILURE);
            }
        }
        token = strtok_r(rest_of_str, delim, &rest_of_str);
    }
    return cnt;
}
