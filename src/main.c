#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

int main() {

    char* cmdline;
    char** arglist;

    while ((cmdline = readline(PROMPT)) != NULL) {

        // Ignore empty commands
        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        // Add command to Readline history
        add_history(cmdline);

        // Handle !n command
        if (cmdline[0] == '!') {

            int n = atoi(cmdline + 1);
            HIST_ENTRY* entry = history_get(n);

            if (entry == NULL) {
                printf("Invalid history number.\n");
                free(cmdline);
                continue;
            }

            free(cmdline);
            cmdline = strdup(entry->line);

            printf("%s\n", cmdline);
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

    printf("\nShell exited.\n");
    return 0;
}
