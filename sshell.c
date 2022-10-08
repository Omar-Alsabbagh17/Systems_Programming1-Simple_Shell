#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#define CMDLINE_MAX 512
# define ARGS_MAX 16


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
                        char* args = strchr(cmd, '-'); // find the first occurence of '-'
                        char * first_command = strtok(cmd, " ");
                        char* args_list[] = {first_command, args, NULL};
                        printf("command: %s\n", first_command);
                        printf("args: %s\n", args);
                        
                        pid_t pid;
                        pid = fork();
                        if (pid == 0) {
                                // child
                                
                                int out = execvp(first_command, args_list ); // if fails, returns -1
                                if (out == -1)
                                        printf("ERROR: command not found\n");
                        }
                        else if (pid > 0)
                        {
                                //parent
                                int status;
                                waitpid(pid, &status, 0); // wait for child to finish
                                fprintf(stderr, "+ completed '%s' [%d]\n", cmd, WEXITSTATUS(status));
                        }
                        else
                        {
                                perror("Error while forking\n");
                                exit(1);
                        }
                }


                //retval = system(cmd);
                //fprintf(stderr, "+ completed '%s' [%d]\n", cmd, retval);
        }

        return EXIT_SUCCESS;
}

