#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

int main() {

    char* cmdline;
    char** arglist;

    while ((cmdline = readline(PROMPT)) != NULL) {

        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        add_history(cmdline);

        if (cmdline[0] == '!') {
            int n = atoi(cmdline + 1);
            HIST_ENTRY* entry = history_get(n);

            if (entry == NULL) {
                printf("Invalid history number.\n");
                free(cmdline);
                continue;
            }

            char* previous = strdup(entry->line);
            free(cmdline);
            cmdline = previous;
            printf("%s\n", cmdline);
        }

        arglist = tokenize(cmdline);

        if (arglist != NULL) {

            int has_operator = 0;

            for (int i = 0; arglist[i] != NULL; i++) {
                if (strcmp(arglist[i], "<") == 0 ||
                    strcmp(arglist[i], ">") == 0 ||
                    strcmp(arglist[i], "|") == 0) {
                    has_operator = 1;
                    break;
                }
            }

            if (!has_operator && handle_builtin(arglist) == 0) {
                execute(arglist);
            }
            else if (has_operator) {
                execute(arglist);
            }

            for (int i = 0; arglist[i] != NULL; i++)
                free(arglist[i]);

            free(arglist);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
