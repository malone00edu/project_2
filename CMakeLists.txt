cmake_minimum_required(VERSION 3.24)
project(project_2 C)

set(CMAKE_C_STANDARD 99)

add_executable(project_2 mysh.c builtinfunctions.c readcommands.c prompts.c freememory.c pathbarenames.c resizearray.c piperedirect.c )

