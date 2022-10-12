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
                        char* cmd_copy = (char*) malloc(strlen(cmd)+1);
                        strcpy(cmd_copy, cmd);
                        char failed = cmd_parser(cmd_copy, &v);
                        if (failed)
                                continue;
                        char is_program = 1; // to distinguish program from argument
                        char is_out_redirection = 0;
                        char is_pip = 0;
                        char finished_input_redirection = 0;
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
                                       
                                        count = 0;
                                        prog.command = v.items[i];
                                        prog.args[count++]=v.items[i];

                                        /* if we reach end of cmd, then it means
                                         it's command without any arguments */
                                        if (i == (v.total-1))
                                        {
                                                retval = execute(&prog, count);
                                                fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);

                                        }
                                        is_program = 0; 
                                }
                                else if (!strcmp(v.items[i], ">"))
                                {
                                        is_out_redirection = 1;
                                        continue;
                                }
                                
                                else if (!strcmp(v.items[i], "<"))
                                {
                                        finished_input_redirection = 1;
                                        prog.args[count++]=v.items[i+1];
                                        retval = execute(&prog, count);
                                        
                                        fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
                                        continue;

                                }
                                else if (finished_input_redirection)
                                {
                                        finished_input_redirection = 0;
                                        is_program = 1;
                                        continue;
                                }
                                else if (!strcmp(v.items[i], "|"))
                                {
                                        
                                        // excute
                                        // store exit status in an array
                                        // redirect 
                                        is_pip = 1;
                                        is_program = 1;
                                        prog.command = NULL;
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
        char found_sub_token = 0;
        
        while (token != NULL)
        {
                // check if we have > or < but without whitespace
                if (strlen(token) > 1)
                        for (unsigned int i = 0; i < strlen(token); ++i)
                        {
                                if (token[i] == '>' || token[i] == '<')
                                {
                                        char delimeter[2] = {token[i], '\0'}; //since strtok doesn't accpet char
                                        found_sub_token = 1;
                                        char* token_copy;
                                        token_copy = (char*) malloc(strlen(token)+1);
                                        //memcpy(token_copy, token, strlen(token));
                                        strcpy(token_copy, token);
                                        char * sub_token = strtok(token_copy, delimeter);
                                        if (i > 0) // it means there was no whitespace before and after
                                        { 
                                                vec_add(v, sub_token);
                                                vec_add(v,delimeter);
                                                sub_token = strtok(NULL, delimeter);
                                                vec_add(v, sub_token);
                                        }
                                        else // it means there was no whitespace only after
                                        {
                                                vec_add(v, delimeter);
                                                vec_add(v, sub_token);

                                        }
                                }
                        }
                
                if (!found_sub_token)
                        vec_add(v, token);
                token = strtok(NULL, " ");
                found_sub_token = 0;
        }
        if (!strcmp(v->items[0], "|") || !strcmp(v->items[0], ">") || !strcmp(v->items[0], "<"))
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
        if (!strcmp(v->items[v->total-1], "<"))
        {
                fprintf(stderr, "ERROR: no input file\n");
                return FAILED; 
        }
        for (int i = 0; i < v->total; i++)
        {
                //printf("%s\n", v->items[i]);
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
                        int fd = open(v->items[i+1] , O_WRONLY | O_CREAT | O_TRUNC, 0777);
                        if (fd == -1)
                        {
                                fprintf(stderr,"ERROR: cannot open output file\n");
                                close(fd);
                                return FAILED;
                        }
                        close(fd);
                }
                if (!strcmp(v->items[i], "<"))
                {
                       
                        int fd = open(v->items[i+1] , O_RDONLY);
                        
                        if (fd == -1)
                        {
                                fprintf(stderr,"ERROR: cannot open input file\n");
                                close(fd);
                                return FAILED;
                        }
                        close(fd);
                }
        }
        // for (int i = 0; i < v->total; i++)
        //         printf("%s\n", v->items[i]);
        return 0;
}


int execute(program * prog, unsigned int num_args)
{
        char* args_list[num_args+1];
        for (unsigned int i = 0; i < num_args; i++)
        {
                args_list[i] = prog->args[i];
                //printf("%s\n", prog->args[i]);
        }
        args_list[num_args]= NULL;
        pid_t pid;
        pid = fork();


        if (pid == 0) {
                // child
                int out = execvp(prog->command,  args_list); // if fails, returns -1
                if (out == -1)
                {
                        printf("ERROR: command not found\n");
                        return 1;
                }
                
        }
        else if (pid != 0)
        {
                //parent
                int status;
                wait(&status); // wait for child to finish
                // fprintf(stderr, "+ completed '%s' [%d]\n", prog->command, WEXITSTATUS(status));
                return WEXITSTATUS(status);

        }
        else
        {
                perror("Error while forking\n");
                //free(v->items);
                exit(1);
        }
        return 0;
}
