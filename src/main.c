#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

int main() {

    char* cmdline;

    while (1) {

        reap_jobs();

        cmdline = readline(PROMPT);

        if (cmdline == NULL)
            break;

        if (cmdline[0] == '\0') {
            free(cmdline);
            continue;
        }

        add_history(cmdline);

        // Handle history execution: !n
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

        // Split commands using semicolon
        char* saveptr;
        char* segment = strtok_r(cmdline, ";", &saveptr);

        while (segment != NULL) {

            char** args = tokenize(segment);

            if (args != NULL) {

                int count = 0;

                while (args[count] != NULL)
                    count++;

                int background = 0;

                // Check for & at the end
                if (count > 0 &&
                    strcmp(args[count - 1], "&") == 0) {

                    background = 1;
                    free(args[count - 1]);
                    args[count - 1] = NULL;
                    count--;
                }

                if (args[0] != NULL) {
                    if (!handle_builtin(args)) {
                        execute(args, background);
                    }
                }

                for (int i = 0; args[i] != NULL; i++)
                    free(args[i]);

                free(args);
            }

            segment = strtok_r(NULL, ";", &saveptr);
        }

        free(cmdline);
    }

    printf("\nShell exited.\n");
    return 0;
}
