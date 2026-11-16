#include "shell.h"

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    char* cmdline = malloc(MAX_LEN);
    if (!cmdline) return NULL;

    if (fgets(cmdline, MAX_LEN, fp) == NULL) {
        free(cmdline);
        return NULL;
    }

    cmdline[strcspn(cmdline, "\n")] = '\0';
    return cmdline;
}

char** tokenize(char* cmdline) {
    if (cmdline == NULL || cmdline[0] == '\0')
        return NULL;

    char** args = calloc(MAXARGS + 1, sizeof(char*));
    int count = 0;
    char* p = cmdline;

    while (*p && count < MAXARGS) {
        while (*p == ' ' || *p == '\t')
            p++;

        if (*p == '\0')
            break;

        if (*p == '<' || *p == '>' || *p == '|') {
            args[count++] = strndup(p, 1);
            p++;
            continue;
        }

        char token[ARGLEN];
        int i = 0;
        char quote = '\0';

        while (*p) {
            if (quote) {
                if (*p == quote) {
                    quote = '\0';
                    p++;
                } else if (i < ARGLEN - 1) {
                    token[i++] = *p++;
                } else {
                    p++;
                }
            } else {
                if (*p == '"' || *p == '\'') {
                    quote = *p++;
                } else if (*p == ' ' || *p == '\t' ||
                           *p == '<' || *p == '>' || *p == '|') {
                    break;
                } else if (i < ARGLEN - 1) {
                    token[i++] = *p++;
                } else {
                    p++;
                }
            }
        }

        token[i] = '\0';

        if (i > 0)
            args[count++] = strdup(token);
    }

    args[count] = NULL;
    return args;
}
