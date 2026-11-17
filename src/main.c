#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_BLOCK_LINES 100

void execute_line(char *line) {
    char *saveptr;
    char *segment = strtok_r(line, ";", &saveptr);

    while (segment != NULL) {
        char **args = tokenize(segment);

        if (args != NULL) {
            int count = 0;

            while (args[count] != NULL)
                count++;

            int background = 0;

            if (count > 0 &&
                strcmp(args[count - 1], "&") == 0) {
                background = 1;
                free(args[count - 1]);
                args[count - 1] = NULL;
            }

            if (args[0] != NULL) {
                if (!handle_builtin(args))
                    execute(args, background);
            }

            for (int i = 0; args[i] != NULL; i++)
                free(args[i]);

            free(args);
        }

        segment = strtok_r(NULL, ";", &saveptr);
    }
}

int execute_condition(char *command) {
    char **args = tokenize(command);

    if (args == NULL || args[0] == NULL) {
        free(args);
        return 1;
    }

    int status;

    if (handle_builtin(args)) {
        status = 0;
    } else {
        status = execute(args, 0);
    }

    for (int i = 0; args[i] != NULL; i++)
        free(args[i]);

    free(args);

    return status;
}

int main() {
    char *cmdline;

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

        /* Handle if-then-else-fi */

        if (strncmp(cmdline, "if ", 3) == 0) {

            char *condition = strdup(cmdline + 3);

            char *then_block[MAX_BLOCK_LINES];
            char *else_block[MAX_BLOCK_LINES];

            int then_count = 0;
            int else_count = 0;

            int in_else = 0;
            int found_then = 0;
            int found_fi = 0;

            free(cmdline);

            while (1) {
                char *line = readline("IF> ");

                if (line == NULL)
                    break;

                if (strcmp(line, "then") == 0) {
                    found_then = 1;
                    free(line);
                    continue;
                }

                if (strcmp(line, "else") == 0) {
                    in_else = 1;
                    free(line);
                    continue;
                }

                if (strcmp(line, "fi") == 0) {
                    found_fi = 1;
                    free(line);
                    break;
                }

                if (in_else) {
                    if (else_count < MAX_BLOCK_LINES)
                        else_block[else_count++] = strdup(line);
                } else {
                    if (then_count < MAX_BLOCK_LINES)
                        then_block[then_count++] = strdup(line);
                }

                free(line);
            }

            if (!found_then || !found_fi) {
                printf("Syntax error: expected then and fi\n");
            } else {
                int status = execute_condition(condition);

                if (status == 0) {
                    for (int i = 0; i < then_count; i++) {
                        execute_line(then_block[i]);
                    }
                } else {
                    for (int i = 0; i < else_count; i++) {
                        execute_line(else_block[i]);
                    }
                }
            }

            /* Free allocated memory */

            for (int i = 0; i < then_count; i++)
                free(then_block[i]);

            for (int i = 0; i < else_count; i++)
                free(else_block[i]);

            free(condition);

            continue;
        }

        execute_line(cmdline);

        free(cmdline);
    }

    printf("\nShell exited.\n");

    return 0;
}
