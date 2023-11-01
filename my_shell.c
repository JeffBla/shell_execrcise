#include <stdlib.h>

#include "include/shell.h"

int main(int argc, char *argv[]) {
    ShellInit();
    ShellLoop();
    return EXIT_SUCCESS;
}
