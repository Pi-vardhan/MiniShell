#include "main.h"

char prompt[20] = "minishell$";
int pid;
char *external_commands[200];
struct process *head = NULL;
char command[50];
int status;
int exit_status = 0;

struct bg_process *bg_head = NULL;
int job_no = 1;

int main()
{
    system("clear");

	signal(SIGINT, signal_handler);
	signal(SIGTSTP, signal_handler);
	signal(SIGCHLD, signal_handler);

    char input_string[50];

    extract_external_commands(external_commands);
    scan_input(prompt, input_string);

    return 0;
}