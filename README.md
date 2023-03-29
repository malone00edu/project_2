# project_2
spring 2023 - cs214 - project 2


**INTRO:** 

This is a basic shell program in C that can run in interactive or batch mode. 

When you enter a command, the program takes the input and separates it into individual commands and their arguments. It then checks if the command is built-in (cd / pwd) or external (ls / cat..)  If it's a built-in command, the program executes it directly using functions like chdir() or getcwd(). If it's an external command, the program creates a child process using fork() and runs the command using execv(). External commands also detects wildcards, redirections, and pipes (more details later).

The program also handles errors that might occur, like typos, incorrect syntax, or file system issues. It prints error messages and exits with an error code if needed. The program frees up any memory that was allocated and starts over again until "exit" is inputed or it reaches the end of the batch file.

<hr>

**DESIGN & IMPLEMENTATION**

**Some key files you may see in this zip:** </br>
header.h -> contains function prototypes and other declarations that are needed by other files in this program  </br>
builtinfunctions.c -> handles functions like pwd and cd </br>
mysh.c -> the main file that sets up data structures and control flow for the shell. Includes a loop that reads input from the user or a batch file and executes the commands.  </br>
readcommands.c -> responsible for reading input from the user or batch file, parsing it into tokens, and storing the tokens in appropriate data structures. </br>
piperedirect.c -> handles execution of nonbuilt in commands, parses commands for redirections/wildcards/pipes </br>


**BATCHMODE - INTERACTIVE MODE**

Both batch mode and interactive mode use a loop to handle parsing and execution of commands. The loop reads input from either a file -batch mode- or from the user -interactive mode- , parses the input into commands and arguments and executes the commands using execv() (non-built in commands).

In batch mode, the program opens the specified file and uses the "lseek" function to get the file size. Memory is allocated dynamically using calloc(). The shell reads the file using the "read" function, stores the commands in memory (using a payload), creates tokens from the commands, and executes them. he allocated memory is freed, once all the commands in the file have been executed.

In interactive mode, the shell takes input commands from the user through the stdin (keyboard). The shell reads the command input, creates tokens from the command, and executes it. Similar to the batch mode, memory is allocated using "calloc" function. This repeats until the user enters the exit command.


**Wildcards:**

The program uses the glob() function to expand wildcard patterns in command arguments. When a wildcard pattern is detected, it calls glob() to generate a list of filenames matching the pattern, and replaces the wildcard argument with the list of filenames.


**Redirections:**

Strcmp() is used to detect whether redirection are found in the instruction, if so, the use_redirection function is called which handles redirection by iterating over the 2D array of command line arguments. If < is detected, it reads input from the file specified, and if > is found, it writes output to the file specified

2D array is use to show each instruction as a sequence of arguments. Each row in the array represents an instruction, and each column in that row represents an argument. Basically its use so that we can execute pipes with redirections easier.

**Pipes:**

Similarly, strcmp() is used to detect if pipes are found in the instructions. Two child processes are created if pipes are detected, which communicate with each other through a pipe. Each child process runs one of the two commands on either side of the pipe. (Note: this program does not support multiple pipes)

<hr> 

**EXTENSIONS:**

**Escape Sequence** 

Escape sequence allows you to treat commands as regular tokens. if you run the code echo hello>world the program will think that > is a command and redirects it to the output world. If you do echo hello\>world, it will treat the > as a regular token, or part of the string. The output will be hello>world.

A switch case is use to handle escape characters such as whitespace, pipes, redirects, if '\' is detected before these character, the escaped character willb be moved to the current position and the rest of the token is shifted to the left to remove the backslash using memmove().

**Home**

Home extension  takes you back to the user's home directory when you 'cd with no arugments.

The builtin_cd function checks if a directory argument is given. If yes, chdir() is used to change directory. If no argument is given, it gets the value of the HOME environment variable and change the directory back to the user's home dir. getenv() is used to retrieve the value of an environment variable - the user's home directory. If the directory does not exist, the function prints an error message.'Sprintf' used to concatenate the home environment variable and the rest of the directory string, from the second character if it starts with a ~. 


<hr>

**TESTING**

First compile the files calling 'make' in linux terminal

**BATCHMODE:**

There is a file call 'myscript.txt' that contain some basic commands such as ls, wildcards, redirections, pipes. Simply type ' ./mysh myscrip.txt ' to test batch mode.


**INTERACTIVE:**

After calling make, type ' ./mysh ' to run the program. Input 'exit' to stop program.

**///USE THESE COMMANDS TO TEST///** <br />

Input: 'pwd' 
<br>Output: should print the current working directory. (testing the built in function)

Input: 'cd' 
<br>Output: should take you back to the home directory. (testing home extension)

Input: 'echo hello\ world' 
<br>Output: should print 'hello world' (testing escape sequence - you can use another cmd instead of echo)

Input: 'echo hello\> world' 
<br>Output: should print 'hello> world'

Input: 'ls' 
<br>Output: should list all files in current directory (could test other basic functions like cat, touch)

Input: 'ls *.c'
<br>Output: should list all files with *.c extensions (testing wildcards)

Input: 'ls > redirect.txt'
<br>Output: should put the results of ls in redirect.txt folder (testing redirects -cat the new file when testing )

Input: 'cat < redirect.txt > output.txt'
<br>Output: read the contents of redirect.txt and put it in output.txt (testing redirects)

Input: 'ls -l | wc'
<br>Output: the output of ls -l is passed as input to wc, which counts the number of lines, words, and bytes in ls -l (testing pipes)

Input: 'ls -l | wc > output.txt'
<br>Output: similar to above but redirect the output to output.txt (testing redirects with pipes)

Input: 'ls *.c > newfile.txt'
<br>Output: Gets all the files with .c extension in teh directory and redirects to newfile.txt (testing redirects with wildcards)

Input: 'ls * /*.c' 
<br>Output: Go to all directories in the current, and prints all the .c files. (use cd .. and then do this command so we guarantee there is a directory for it to look at)

Input: 'exit'
<br>Output: Exits the program!


--To test errors prompts, input wrong file names etc--





