cmake_minimum_required(VERSION 3.26)
project(MyShell C)

set(CMAKE_C_STANDARD 11)

include_directories(include)

add_executable(my_shell
        include/builtin.h
        include/command.h
        include/shell.h
        src/builtin.c
        src/command.c
        src/shell.c
        my_shell.c)
