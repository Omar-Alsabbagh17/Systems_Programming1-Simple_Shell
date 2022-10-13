# Simple Shell 

## Objective
This project implements a simple command-line interpreter called sshell which accepts commands from the user and excetes them. This shell supports ouput and input redirect and pipeline commands.

## Implementation
1. Get the command input from the user using fgets().
2. Check if the input command is a builtin command (pwd, cd , exit) or a regular command.
3. Parse the input command to check for whitespaces
4. Parse the string to select the commands, arguments, redirection or piping of command line. 
5. fork(), exec(), wait() for the command and display the oput message. 

## fork(), exec(), wait()
To execute the diffent commands entered by the user, we call the evecute function which take in the program struct as one of its arguments. The program struct holds one command and argumments of the user input command. The parent (shell) then creates a new process by calling fork and waits for child to come back to print its exit value. The child process calls exec() and exectues the command for the user. The struct values then get set to NULL as the shell waits for a new command to be entered. 

## Output Redirection and Pipelined Commands
While parsing the command to select its command and arguments, if we encounter the metacharacter 
1. '>' means output redirect. We turn on the output redirect flag. 
2. '|' means pipeline commands. We turn on the pipline commands flag. 
3. '<' means input redirect. We turn on the iput redirect flag. 

We use the dup2() system calls to manipulate the file descriptor table to either read or write from a particular file or a pipe.

## Directory Stack
This shell also supports directory stack where 
1. pushd pushes the current directory to a stack before changing the directory.
    1. Turn on the pushd flag. 
    2. Get working directory path.
    3. Add it to vec_add.
    4. Print completion message.
2. popd pops the latest directory that was pushed onto the stack, if any, and changes back to it.
    1. Pop the directory.
    2. Print completion message.
3. dirs lists the stack of remembered directories.
    1. Traverse the directory stack and print the directories. 
    2. Print completion message.
