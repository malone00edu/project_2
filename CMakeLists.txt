cmake_minimum_required(VERSION 3.24)
project(project_2 C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -fno-omit-frame-pointer -Wunused-variable -g -O1")

add_executable(project_2 mysh.c builtinfunctions.c readcommands.c prompts.c freememory.c piperedirect.c)

