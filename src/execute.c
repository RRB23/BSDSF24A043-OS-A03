#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

int handle_builtin(char** arglist) {

    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    else if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL) {
            fprintf(stderr, "cd: directory required\n");
        }
        else if (chdir(arglist[1]) != 0) {
            perror("cd");
        }
        return 1;
    }

    else if (strcmp(arglist[0], "help") == 0) {
        printf("\nBuilt-in Commands:\n");
        printf("cd <directory> - Change directory\n");
        printf("exit           - Exit shell\n");
        printf("help           - Display help\n");
        printf("jobs           - Display job status\n");
        printf("history        - Display command history\n\n");
        return 1;
    }

    else if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    else if (strcmp(arglist[0], "history") == 0) {
        HIST_ENTRY** entries = history_list();

        if (entries != NULL) {
            for (int i = 0; entries[i] != NULL; i++) {
                printf("%d  %s\n", i + 1, entries[i]->line);
            }
        }
        return 1;
    }

    return 0;
}

int execute(char* arglist[]) {

    int status;
    int cpid = fork();

    switch (cpid) {
        case -1:
            perror("fork failed");
            return -1;

        case 0:
            execvp(arglist[0], arglist);
            perror("Command not found");
            exit(1);

        default:
            waitpid(cpid, &status, 0);
            return 0;
    }
}
