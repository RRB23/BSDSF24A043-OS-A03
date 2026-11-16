#include "shell.h"

char* command_history[HISTORY_SIZE];
int history_count = 0;

void add_history(char* cmd) {

    if (history_count == HISTORY_SIZE) {
        free(command_history[0]);

        for (int i = 1; i < HISTORY_SIZE; i++) {
            command_history[i - 1] = command_history[i];
        }

        history_count--;
    }

    command_history[history_count++] = strdup(cmd);
}

int main() {

    char* cmdline;
    char** arglist;

    while ((cmdline = read_cmd(PROMPT, stdin)) != NULL) {

        // Handle !n command
        if (cmdline[0] == '!') {

            int n = atoi(cmdline + 1);

            if (n < 1 || n > history_count) {
                printf("Invalid history number.\n");
                free(cmdline);
                continue;
            }

            free(cmdline);
            cmdline = strdup(command_history[n - 1]);

            printf("%s\n", cmdline);
        }

        // Store command in history
        if (cmdline[0] != '\0') {
            add_history(cmdline);
        }

        arglist = tokenize(cmdline);

        if (arglist != NULL) {

            if (!handle_builtin(arglist)) {
                execute(arglist);
            }

            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }

            free(arglist);
        }

        free(cmdline);
    }

    for (int i = 0; i < history_count; i++) {
        free(command_history[i]);
    }

    printf("\nShell exited.\n");

    return 0;
}
