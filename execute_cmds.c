#include "main.h"

extern int exit_status;

void execute_internal_commands(char *input_string)
{
    char *cmd = strtok(input_string, " ");
    char *arg = NULL;

    if(strcmp(cmd, "exit") == 0)
    {
        exit(0);
    }
    else if(strcmp(cmd, "cd") == 0)
    {
        arg = strtok(NULL, " ");

        if(arg == NULL)
        {
            printf(ANSI_COLOR_RED "cd: missing argument\n" ANSI_COLOR_RESET);
        }
        else if(chdir(arg) == -1)
        {
            perror("cd");
        }
    }
    else if(strcmp(cmd, "pwd") == 0)
    {
        char cwd[100];

        if(getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
        }
        else
        {
            perror("pwd");
        }
    }
    else if(strcmp(cmd, "jobs") == 0)
    {
        print_jobs();
        print_bg_jobs();
    }
    else if(strcmp(cmd, "fg") == 0)
    {
        fg();
    }
    else if(strcmp(cmd, "bg") == 0)
    {
        bg();
    }
    else if(strcmp(cmd, "echo") == 0)
    {
        arg = strtok(NULL, " ");

        int first = 1;

        while(arg != NULL)
        {
            if(!first)
                printf(" ");

            if(strcmp(arg, "$?") == 0)
            {
                printf("%d", exit_status);
            }
            else if(strcmp(arg, "$$") == 0)
            {
                printf("%d", getpid());
            }
            else if(strcmp(arg, "$SHELL") == 0)
            {
                printf("%s", getenv("SHELL"));
            }
            else
            {
                printf("%s", arg);
            }

            first = 0;
            arg = strtok(NULL, " ");
        }

        printf("\n");
    }
}

void execute_pipes(char *input_string)
{
    char *commands[20];
    int count = 0;

    commands[count] = strtok(input_string, "|");

    while(commands[count] != NULL && count < 19)
    {
        count++;
        commands[count] = strtok(NULL, "|");
    }

    if(count < 2)
        return;

    int pipefd[2];
    int prev_fd = -1;
    pid_t pids[20];

    for(int i = 0; i < count; i++)
    {
        if(i < count - 1)
        {
            if(pipe(pipefd) == -1)
            {
                perror("pipe");
                return;
            }
        }

        pids[i] = fork();

        if(pids[i] < 0)
        {
            perror("fork");
            return;
        }

        if(pids[i] == 0)
        {
            
            if(prev_fd != -1)
            {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }

            if(i < count - 1)
            {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[1]);
            }

            char *argv[20];
            int j = 0;

            argv[j] = strtok(commands[i], " ");

            while(argv[j] != NULL && j < 19)
            {
                j++;
                argv[j] = strtok(NULL, " ");
            }

            execvp(argv[0], argv);

            perror("execvp");
            exit(1);
        }

        
        if(prev_fd != -1)
            close(prev_fd);

        if(i < count - 1)
        {
            close(pipefd[1]);
            prev_fd = pipefd[0];
        }
    }

    if(prev_fd != -1)
        close(prev_fd);

    int status;

    for(int i = 0; i < count; i++)
    {
        if(i == count - 1)
        {
            waitpid(pids[i], &status, 0);
        }
        else
        {
            waitpid(pids[i], NULL, 0);
        }
    }

    if(WIFEXITED(status))
    {
        exit_status = WEXITSTATUS(status);
    }
    else if(WIFSIGNALED(status))
    {
        exit_status = 128 + WTERMSIG(status);
    }
}