#include "main.h"

extern char *external_commands[200];

int check_command_type(char *command)
{  

    char *builtins[] = { "cd", "pwd", "exit", "jobs", "fg", "bg","echo", NULL };                 

    // Check builtin 
    for (int i = 0; builtins[i] != NULL; i++)
    {
        if (strcmp(builtins[i], command) == 0)
        {
            return BUILTIN;
        }
    }

    // Check external command 
    for (int i = 0; external_commands[i] != NULL; i++)
    {
        if (strcmp(external_commands[i], command) == 0)
        {
            return EXTERNAL;
        }
    }

    return NO_COMMAND;
}


void extract_external_commands(char **external_commands)
{
    FILE *fp = fopen("external_commands.txt", "r");

    if (fp == NULL)
    {
        perror("fopen");
        return;
    }

    char command[50];
    int i = 0;

    while (fgets(command, sizeof(command), fp) != NULL)
    {
        // Remove newline 
        command[strcspn(command, "\n")] = '\0';

        // Allocate memory 
        external_commands[i] = malloc(strlen(command) + 1);

        if (external_commands[i] == NULL)
        {
            perror("malloc");
            break;
        }

        // Copy command 
        strcpy(external_commands[i], command);

        i++;

        if (i == 199)
        {
            break;
        }
    }

    external_commands[i] = NULL;

    fclose(fp);
}