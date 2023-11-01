#include "../include/builtin.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>

void hello() {
    printf("=======================================================\n"
           "* Wellcom to my little shell:                         *\n"
           "*                                                     *\n"
           "* Type 'help' to see builtin functions.               *\n"
           "* + redirection: '>' or '<'                           *\n"
           "* + pipe: '|'                                         *\n"
           "* + background: '&'                                   *\n"
           "* Make sure they are seperated by '(space)'  .        *\n"
           "*                                                     *\n"
           "* Have fun!!                                          *\n"
           "=======================================================\n");
}

int ShellHelp(char **args) {
    printf("-------------------------------------------------------\n"
           "my little shell\n"
           "Type program names and arguments, and hit enter.\n"
           "\n"
           "1: help:    show all build-in function info\n"
           "2: cd:      change directory\n"
           "3: echo:    echo the strings to standard output\n"
           "4: record:  show last-16 cmds showed in record\n"
           "5: replay: re-execute the cmd showed in record\n"
           "6: mypid: find and print process-ids\n"
           "7: exit: exit shell\n"
           "\n"
           "Use the 'man' command for infomation on other programs.\n"
           "-------------------------------------------------------\n");
    return 1;
}

int ShellCd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "ShellCd: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("ShellCd: chdir");
        }
    }
    return 1;
}

int ShellEcho(char **args) {
    int start_i = 1;
    char end_char = '\n';
    if (strcmp(args[1], "-n") == 0) {
        start_i = 2;
        end_char = '\0';
    }
    for (; args[start_i] != NULL; start_i++) {
        printf("%s ", args[start_i]);
    }
    printf("\b%c", end_char);
    return 1;
}

int ShellRecord(char **args) {
    printf("history cmd:\n");
    for (int i = 1, curr = histoty_recent_index;
         i <= HISTORY_SIZE && history[curr] != NULL; i++) {
        printf("%2d: %s\n", i, history[curr]);
        curr = (curr - 1 + HISTORY_SIZE) % HISTORY_SIZE;
    }
    printf("\n");
    return 1;
}

int ShellReplay(char **args) {
    if (args[1] != NULL) {
        char *endptr;
        int id = strtol(args[1], &endptr, 10);
        if (*endptr != '\0') {
            fprintf(stderr, "replay: wrong args\n");
        } else {
            if (id >= 1 && id <= 16) {
                int exec_index = (histoty_recent_index - id + 1 + HISTORY_SIZE) % HISTORY_SIZE;
                InputBufferInsert(history[exec_index]);
            } else {
                fprintf(stderr, "replay: wrong args\n");
            }
        }
    } else {
        printf("Please enter 1~16 to replay the command.\n");
    }
    return 1;
}

int ShellMypid(char **args) {
    int target_pid, fd, status;
    char *endptr, *token, *save_ptr;
    char pathName[BUFFER_SIZE], read_buffer[BUFFER_SIZE];
    if (strcmp(args[1], "-i") == 0) {
        printf("%d\n", getpid());
    } else if (strcmp(args[1], "-p") == 0) {
        if (args[2] != NULL) {
            target_pid = strtol(args[2], &endptr, 10);
            if (*endptr != '\0') {
                fprintf(stderr, "ShellMypid: wrong args\n");
                return 1;
            }
        } else {
            fprintf(stderr, "ShellMypid: wrong args\n");
            return 1;
        }
        // read /proc/{pid}
        sprintf(pathName, "/proc/%d/stat", target_pid);
        fd = open(pathName, O_RDONLY);
        status = read(fd, read_buffer, BUFFER_SIZE * sizeof(char));
        if (status == -1) {
            perror("ShellMypid -c: read error");
        }
        // parse the buffer
        token = strtok_r(read_buffer, " ", &save_ptr);
        for (int i = 3; i > 0; i--) {
            token = strtok_r(save_ptr, " ", &save_ptr);
        }
        printf("%s\n", token);
    } else if (strcmp(args[1], "-c") == 0) {
        {
//            // fast look for the shell's child
//            int status;
//            Node *curr = childProcsList.head, *prev = NULL;
//            while (curr != NULL) {
//                pid_t wpid = waitpid(curr->pid, &status, WNOHANG);
//                prev = curr;
//                curr = curr->next;
//                if (wpid == 0) {
//                    // wpid not finish yet
//                    printf("%d\n", prev->pid);
//                } else {
//                    // wpid finished
//                    ChildProcsRemove(prev);
//                }
//            }
//            if (childProcsList.sz == 0) {
//                printf("Empty\n");
//            }
        }
        if (args[2] != NULL) {
            target_pid = strtol(args[2], &endptr, 10);
            if (*endptr != '\0') {
                fprintf(stderr, "ShellMypid: wrong args\n");
                return 1;
            }
        } else {
            fprintf(stderr, "ShellMypid: wrong args\n");
            return 1;
        }
        pid_t childPid, parentPid;
        DIR *dp = opendir("/proc");
        struct dirent *dirp;
        while ((dirp = readdir(dp)) != NULL) {
            childPid = strtol(dirp->d_name, &endptr, 10);
            if (*endptr != '\0') {
                continue;
            }
            sprintf(pathName, "/proc/%d/stat", childPid);
            fd = open(pathName, O_RDONLY);
            status = read(fd, read_buffer, BUFFER_SIZE * sizeof(char));
            if (status == -1) {
                perror("ShellMypid -c: read error");
            }
            // parse the buffer
            token = strtok_r(read_buffer, " ", &save_ptr);
            for (int i = 3; i > 0; i--) {
                token = strtok_r(save_ptr, " ", &save_ptr);
            }
            parentPid = strtol(token, &endptr, 10);
            if (parentPid == target_pid ){
                printf("%d\n", childPid);
            }
        }
        closedir(dp);
    }
    return 1;
}

int ShellExit(char **args) {
    printf("my little shell: see you next time.\n");
    return 0;
}

char *builtin_strCmd[] = {
        "help",
        "cd",
        "echo",
        "record",
        "replay",
        "mypid",
        "exit"
};

int (*builtin_func[])(char **) = {
        &ShellHelp,
        &ShellCd,
        &ShellEcho,
        &ShellRecord,
        &ShellReplay,
        &ShellMypid,
        &ShellExit
};

int NumBuiltins() {
    return sizeof(builtin_strCmd) / sizeof(builtin_strCmd[0]);
}