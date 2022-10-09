#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#include "string_vector.h"

#define CMDLINE_MAX 512
# define ARGS_MAX 16
# define FAILED 1

typedef struct
{
        char* command;
        char* args[ARGS_MAX];
} program;


char cmd_parser(char* , string_vector*); // returns an array of program
int execute(program * , unsigned int ); 

int main(void)
{
        char cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
                int retval;

                /* Print prompt */
                printf("sshell@ucd$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';
                
                char* is_cd_command = strstr(cmd, "cd"); // check if it's cd command
                
                /* ======== Builtin command ===============*/
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, 0);
                        return EXIT_SUCCESS;
                        //break;
                }
                else if (!strcmp(cmd, "pwd"))
                {
                        char cwd[CMDLINE_MAX];
                        if (getcwd(cwd, sizeof(cwd)) != NULL) 
                        {
                                // executed sucessfully
                                printf("%s\n", cwd);
                                fprintf(stderr, "+ completed 'pwd' [0]\n");
                        } 
                        else
                        {
                                //perror("Error in getcwd()");
                                fprintf(stderr,"Error in getcwd()");
                                fprintf(stderr, "+ completed 'pwd' [1]\n");
                        }
                }

                else if (is_cd_command)
                {
                        char * new_dir = strtok(cmd, " ");
                        new_dir = strtok(NULL, " ");
                        if (!strcmp(new_dir, "~"))
                                // since chdir doesn't recognize ~, we have to use getenv
                                new_dir = getenv("HOME");
                        if (chdir(new_dir) != 0)
                        {
                                fprintf(stderr,"Error: cannot cd into directory\n");
                                fprintf(stderr, "+ completed '%s' [%d]\n", cmd, 1);
                        }
                        else
                        {
                                fprintf(stderr, "+ completed '%s' [%d]\n", cmd, 0);
                        }
                }
                /* ============ Regular command  =====================================================*/
                
                else{
                        string_vector v;
                        vec_init(&v);
                        char cmd_copy[strlen(cmd)+1];
                        strcpy(cmd_copy, cmd);
                        char failed = cmd_parser(cmd_copy, &v);
                        if (failed)
                                continue;
                        char is_program = 1; // to distinguish program from argument
                        char is_out_redirection = 0;
                        char is_pip = 0;
                        unsigned int count = 0; // to keep track of arguments
                        program prog;
                        for (int i = 0; i < v.total; i++)
                        {
                                if (count >= ARGS_MAX)
                                {
                                        fprintf(stderr, "ERROR: too many arguments\n");
                                        break;

                                }
                                if (is_program)
                                {
                                        // check if it's a valid command
                                        char temp_cmd[strlen(v.items[i])+30];
                                        strcpy( temp_cmd, v.items[i]);
                                        strcat(temp_cmd, " > /dev/null 2>&1");
                                        int ret = system(temp_cmd);
                                        if (ret != 0)
                                        {
                                                printf("ERROR: command not found\n");
                                                break;
                                        }

                                        count = 0;
                                        prog.command = v.items[i];
                                        prog.args[count++]=v.items[i];
                                        is_program = 0; 
                                }
                                else if (!strcmp(v.items[i], ">"))
                                {
                                        is_out_redirection = 1;
                                        continue;

                                }
                                else if (!strcmp(v.items[i], "|"))
                                {
                                        is_pip = 1;
                                        is_program = 1;
                                        continue;

                                }
                                else if (i == (v.total-1)) // reached end of cmd
                                {
                                        if (is_out_redirection)
                                        {       int fd = open(v.items[i], O_RDWR);
                                                int screen_stdout = dup(STDOUT_FILENO);
                                                dup2(fd, STDOUT_FILENO);
                                                retval = execute(&prog, count);
                                                // restore stdout back to terminal
                                                dup2(screen_stdout, STDOUT_FILENO);
                                                close(screen_stdout);
                                                close(fd);
                                                fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
                                                
                                        }
                                        else
                                        {
                                                prog.args[count++]=v.items[i];
                                                retval = execute(&prog, count);
                                                fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
                                        }
                                        break;

                                }
                                else
                                {
                                        prog.args[count++]=v.items[i];
                                }
                                
                        }

                        free(v.items);
                }


                //retval = system(cmd);
                //fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
        }

        return EXIT_SUCCESS;
}

char cmd_parser(char* cmd, string_vector* v)
{
        char * token = strtok(cmd, " ");
        
        while (token != NULL)
        {
                //printf("%s\n", token);
                vec_add(v, token);
                token = strtok(NULL, " ");
        }
        if (!strcmp(v->items[0], "|") || !strcmp(v->items[0], ">"))
        {
                fprintf(stderr, "ERROR: missing command\n");
                return FAILED; 
        }
        if (!strcmp(v->items[v->total-1], "|"))
        {
                fprintf(stderr, "ERROR: missing command\n");
                return FAILED; 
        }
        if (!strcmp(v->items[v->total-1], ">"))
        {
                fprintf(stderr, "ERROR: no output file\n");
                return FAILED; 
        }
        for (int i = 0; i < v->total; i++)
        {
                if (!strcmp(v->items[i], ">"))
                {
                        for (int j = i+1; j < v->total; j++)
                        {
                                if (!strcmp(v->items[j], "|"))
                                {
                                        fprintf(stderr, "ERROR: mislocated output redirection\n");
                                        return FAILED;
                                }
                        }
                        int fd = open(v->items[i+1] , O_WRONLY | O_CREAT, 0777);
                        if (fd == -1)
                        {
                                fprintf(stderr,"ERROR: cannot open output file\n");
                                close(fd);
                                return FAILED;
                        }
                        close(fd);
                }
        }
        return 0;
}


int execute(program * prog, unsigned int num_args)
{
        char* args_list[num_args+1];
        for (int i = 0; i < num_args; i++)
        {
                args_list[i] = prog->args[i];
                //printf("%s\n", args_list[i]);
        }
        args_list[num_args]= NULL;
        pid_t pid;
        pid = fork();

        if (pid == 0) {
                // child
                int out = execvp(prog->command,  args_list); // if fails, returns -1
        }
        else if (pid > 0)
        {
                //parent
                int status;
                waitpid(pid, &status, 0); // wait for child to finish
                // fprintf(stderr, "+ completed '%s' [%d]\n", prog->command, WEXITSTATUS(status));
                return WEXITSTATUS(status);

        }
        else
        {
                perror("Error while forking\n");
                //free(v->items);
                exit(1);
        }
}
