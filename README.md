# project_2
spring 2023 - cs214 - project 2


Intro:
This is a basic shell program in C that can run in interactive or batch mode. 

When you enter a command, the program takes the input and separates it into individual commands and their arguments. It then checks if the command is built-in (cd / pwd) or external (ls / cat..)  If it's a built-in command, the program executes it directly using functions like chdir() or getcwd(). If it's an external command, the program creates a child process using fork() and runs the command using execv().

The program also handles errors that might occur, like typos, incorrect syntax, or file system issues. It prints error messages and exits with an error code if needed. The program frees up any memory that was allocated and starts over again until "exit" is inputed or it reaches the end of the batch file.


IMPLEMENTATION


Wildcards:



Redirections:



Pipes:



TESTING
