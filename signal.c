#include "main.h"

extern char prompt[20];
extern int pid;
extern char command[50];
extern struct process *head;

extern struct bg_process *bg_head;
extern int job_no;

void signal_handler(int signum)
{
    if(signum == SIGCHLD)
    {
        struct bg_process *temp = bg_head;
        int child_status;

        while(temp != NULL)
        {
            if(waitpid(temp->pid, &child_status, WNOHANG) > 0)
            {
                if(WIFEXITED(child_status))
                {
                    printf(ANSI_COLOR_YELLOW "[%d] Done %s (exit %d)\n" ANSI_COLOR_RESET, temp->job_no, temp->cmd,WEXITSTATUS(child_status));
                }
                else if(WIFSIGNALED(child_status))
                {
                    printf(ANSI_COLOR_YELLOW "[%d] Terminated %s (signal %d)\n" ANSI_COLOR_RESET, temp->job_no, temp->cmd, WTERMSIG(child_status));
                }

                int finished_pid = temp->pid;
                temp = temp->link;

                remove_bg_process(finished_pid);
            }
            else
            {
                temp = temp->link;
            }
        }

        return;
    }

    if(pid == 0)
    {
        if(signum == SIGINT)
        {
            printf("\n");
            printf(ANSI_COLOR_MAGENTA "%s " ANSI_COLOR_RESET, prompt);
            fflush(stdout);
        }
    }
    else
    {
        if(signum == SIGTSTP)
        {
            insert_at_first(pid, command);

            printf(ANSI_COLOR_YELLOW"\n[%d] Stopped %s\n" ANSI_COLOR_RESET,
                   pid, command);
            fflush(stdout);
        }
    }
}

void insert_at_first(int child_pid, char *cmd)
{
	struct process *new = malloc(sizeof(struct process));

	if(new == NULL)
	{
		perror("malloc");
		return;
	}

	new->pid = child_pid;

	new->cmd = malloc(strlen(cmd) + 1);

	if(new->cmd == NULL)
	{
		perror("malloc");
		free(new);
		return;
	}

	strcpy(new->cmd, cmd);

	new->link = head;

	head = new;
}

void print_jobs(void)
{
    struct process *temp = head;

    while(temp != NULL)
    {
        printf(ANSI_COLOR_YELLOW "[%d] Stopped %s\n" ANSI_COLOR_RESET, temp->pid, temp->cmd);
        temp = temp->link;
    }
}

void fg(void)
{
    if(head == NULL)
    {
        printf("fg: no stopped jobs\n");
        return;
    }

    kill(head->pid, SIGCONT);

    waitpid(head->pid, NULL, WUNTRACED);

    struct process *temp = head;
    head = head->link;

    free(temp->cmd);
    free(temp);
}

void bg(void)
{
    if(head == NULL)
    {
        printf("bg: no stopped jobs\n");
        return;
    }

    int child_pid = head->pid;

    char *cmd = malloc(strlen(head->cmd) + 1);

    if(cmd == NULL)
    {
        perror("malloc");
        return;
    }

    strcpy(cmd, head->cmd);

    kill(child_pid, SIGCONT);

    insert_bg_process(child_pid, cmd);

    free(cmd);

    delete_at_first();
}

void delete_at_first(void)
{
    if(head == NULL)
        return;

    struct process *temp = head;

    head = head->link;

    free(temp->cmd);
    free(temp);
}

void insert_bg_process(int child_pid, char *cmd)
{
    struct bg_process *new = malloc(sizeof(struct bg_process));

    if(new == NULL)
    {
        perror("malloc");
        return;
    }

    new->pid = child_pid;
    new->job_no = job_no++;

    new->cmd = malloc(strlen(cmd) + 1);

    if(new->cmd == NULL)
    {
        perror("malloc");
        free(new);
        return;
    }

    strcpy(new->cmd, cmd);

    new->link = bg_head;
    bg_head = new;
}

void print_bg_jobs(void)
{
    struct bg_process *temp = bg_head;

    while(temp != NULL)
    {
        printf(ANSI_COLOR_YELLOW "[%d] %d Running %s\n" ANSI_COLOR_RESET,
               temp->job_no,
               temp->pid,
               temp->cmd);

        temp = temp->link;
    }
}

void remove_bg_process(int child_pid)
{
    struct bg_process *temp = bg_head;
    struct bg_process *prev = NULL;

    while(temp != NULL)
    {
        if(temp->pid == child_pid)
        {
            if(prev == NULL)
            {
                bg_head = temp->link;
            }
            else
            {
                prev->link = temp->link;
            }

            free(temp->cmd);
            free(temp);
            return;
        }

        prev = temp;
        temp = temp->link;
    }
}