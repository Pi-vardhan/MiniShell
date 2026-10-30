#include "main.h"

extern int pid;
extern char command[50];
extern int status;
extern int exit_status;

extern struct bg_process *bg_head;
extern int job_no;

void scan_input(char *prompt, char *input_string)
{

    signal(SIGINT, signal_handler);
    signal(SIGTSTP, signal_handler);
    signal(SIGCHLD, signal_handler);

    while (1)
    {
        printf(ANSI_COLOR_MAGENTA "%s " ANSI_COLOR_RESET, prompt);
        
        if(fgets(input_string, 50, stdin) == NULL)
        {
            printf("\n");
            exit(0);
        }
        input_string[strcspn(input_string, "\n")] = '\0';

        // Empty input 
        if (strlen(input_string) == 0)
        {
            continue;
        }

        
        //Check whether input is PS1=
     
        if (strncmp(input_string, "PS1=", 4) == 0)
        {
            if (input_string[4] == ' ')
            {
                printf(ANSI_COLOR_RED "%s: command not found\n" ANSI_COLOR_RESET, input_string);
            }
            else
            {
                strcpy(prompt, input_string + 4);
            }
        }
        else
        {
            int background = 0;

            int len = strlen(input_string);

            if(len > 0 && input_string[len - 1] == '&')
            {
                background = 1;
                input_string[len - 1] = '\0';

                while(len > 1 && input_string[len - 2] == ' ')
                {
                    input_string[len - 2] = '\0';
                    len--;
                }
            }

            if(strchr(input_string, '|') != NULL)
            {
                execute_pipes(input_string);
                continue;
            }

            char *cmd = get_command(input_string);

            if(strcmp(cmd, "$?") == 0)
            {
                printf("%d\n", WEXITSTATUS(status));
                continue;
            }

            int ret = check_command_type(cmd);

            
            if(ret == BUILTIN)
            {
                execute_internal_commands(input_string);
            }
            else if(ret == EXTERNAL)
            {
                strcpy(command, input_string);
                pid = fork();

                if(pid < 0)
                {
                    perror("fork");
                    pid = 0;
                }
                else if(pid == 0)
                {
                    // Child process 

                    signal(SIGINT, SIG_DFL);
                    signal(SIGTSTP, SIG_DFL);

                    char *argv[20];
                    int i = 0;

                    argv[i] = strtok(input_string, " ");

                    while(argv[i] != NULL)
                    {
                        i++;
                        argv[i] = strtok(NULL, " ");
                    }

                    execvp(argv[0], argv);

                    perror("execvp");
                    exit(1);

                    
                }
                else
                {
                    if(background)
                    {
                        insert_bg_process(pid, command);

                        printf("[%d] %d\n", job_no - 1, pid);

                        pid = 0;
                    }
                    else
                    {
                        waitpid(pid, &status, WUNTRACED);

                        if(WIFEXITED(status))
                        {
                            exit_status = WEXITSTATUS(status);
                        }
                        else if(WIFSIGNALED(status))
                        {
                            exit_status = 128 + WTERMSIG(status);
                        }

                        if(!WIFSTOPPED(status))
                        {
                            pid = 0;
                        }
                    }
                }
            }
            else
            {
                printf(ANSI_COLOR_YELLOW "NO COMMAND\n" ANSI_COLOR_RESET);
                exit_status = 127;
            }
        
        }
    }
}

char *get_command(char *input_string)
{
    static char command[20];

    int i = 0;

    // Skip leading spaces 
    while (input_string[i] == ' ')
    {
        i++;
    }

    int j = 0;

    // Copy first word 
    while (input_string[i] != ' ' && input_string[i] != '\0')
    {
        command[j++] = input_string[i++];
    }

    command[j] = '\0';

    return command;
}
