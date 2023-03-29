# project_2
spring 2023 - cs214 - project 2


**Intro:**

This is a basic shell program in C that can run in interactive or batch mode. 

When you enter a command, the program takes the input and separates it into individual commands and their arguments. It then checks if the command is built-in (cd / pwd) or external (ls / cat..)  If it's a built-in command, the program executes it directly using functions like chdir() or getcwd(). If it's an external command, the program creates a child process using fork() and runs the command using execv(). External commands also detects wildcards, redirections, and pipes (more details later).

The program also handles errors that might occur, like typos, incorrect syntax, or file system issues. It prints error messages and exits with an error code if needed. The program frees up any memory that was allocated and starts over again until "exit" is inputed or it reaches the end of the batch file.


**DESIGN & IMPLEMENTATION**

**Some key files you may see in this zip:** </br>
header.h -- contains function prototypes and other declarations that are needed by other files in this program  </br>
builtinfunctions.c -- handles functions like pwd and cd </br>
mysh.c -- the main file that sets up data structures and control flow for the shell. Includes a loop that reads input from the user or a batch file and executes the commands.  </br>
readcommands.c -- responsible for reading input from the user or batch file, parsing it into tokens, and storing the tokens in appropriate data structures. </br>
redirects.c -- handles execution of nonbuilt in commands, parses commands for redirections/wildcards/pipes </br>


**/* BATCHMODE/INTERACTIVE MODE: */**

Both batch mode and interactive mode use a loop to handle parsing and execution of commands. The loop reads input from either a file -batch mode- or from the user -interactive mode- , parses the input into commands and arguments and executes the commands using execv() (non-built in commands).

In batch mode, the program opens the specified file and uses the "lseek" function to get the file size. Memory is allocated dynamically using calloc(). The shell reads the file using the "read" function, stores the commands in memory (using a payload), creates tokens from the commands, and executes them. he allocated memory is freed, once all the commands in the file have been executed.

In interactive mode, the shell takes input commands from the user through the stdin (keyboard). The shell reads the command input, creates tokens from the command, and executes it. Similar to the batch mode, memory is allocated using "calloc" function. This repeats until the user enters the exit command.


**/* Wildcards: */**

The program uses the glob() function to expand wildcard patterns in command arguments. When a wildcard pattern is detected, it calls glob() to generate a list of filenames matching the pattern, and replaces the wildcard argument with the list of filenames.


**/* Redirections: */**


**/* Pipes: */**


**EXTENSIONS:**

**/* HOME */**

Essentially if when you 'cd' with no arguments, the program takes you back to the user's home directory. 

The builtin_cd function checks if a directory argument is given. If yes, chdir() is used to change directory. If no argument is given, it gets the value of the HOME environment variable and change the directory back to the user's home dir. getenv() is used to retrieve the value of an environment variable, in this case, the HOME environment variable which represents the user's home directory. If the directory does not exist, the function prints an error message. 





**TESTING**

First compile the files calling 'make' in linux terminal, and then /mysh to run the program.

**/* BATCHMODE: */**

There is a file call 'myscript.txt' that contain some basic commands such as ls, wildcards, redirections, pipes. Simply type ' ./mysh myscrip.txt ' to test batch mode.


**/* INTERACTIVE: */**

After calling make, type ./mysh to run the program.

**///USE THESE COMMANDS TO TEST///** <br />
pwd <br />
cd ../     (to test if the home extension work just cd with no arguments and ls)  <br />
ls <br />
cd  <br />
cat mysh.c <br />
ls *.c <br />
ls m*h.c  <br />
ls > redirec.txt <br />
cat < redirec.txt > redirec2.txt  <br />
ls | sort <br />
ls -l | wc <br />







