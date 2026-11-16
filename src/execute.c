#include "shell.h"

int handle_builtin(char** arglist) {

    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL)
            fprintf(stderr, "cd: directory required\n");
        else if (chdir(arglist[1]) != 0)
            perror("cd");
        return 1;
    }

    if (strcmp(arglist[0], "help") == 0) {
        printf("\nBuilt-in Commands:\n");
        printf("cd <directory> - Change directory\n");
        printf("exit           - Exit shell\n");
        printf("help           - Display help\n");
        printf("jobs           - Display job status\n");
        printf("history        - Display command history\n\n");
        return 1;
    }

    if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    if (strcmp(arglist[0], "history") == 0) {
        printf("History is handled by Readline.\n");
        return 1;
    }

    return 0;
}

static void run_command(char** args, int input_fd, int output_fd) {

    if (input_fd != STDIN_FILENO) {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }

    if (output_fd != STDOUT_FILENO) {
        dup2(output_fd, STDOUT_FILENO);
        close(output_fd);
    }

    int in = -1, out = -1;

    for (int i = 0; args[i] != NULL; i++) {

        if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Input filename missing\n");
                exit(1);
            }

            in = open(args[i + 1], O_RDONLY);
            if (in < 0) {
                perror("open input");
                exit(1);
            }

            args[i] = NULL;
        }

        else if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Output filename missing\n");
                exit(1);
            }

            out = open(args[i + 1],
                       O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (out < 0) {
                perror("open output");
                exit(1);
            }

            args[i] = NULL;
        }
    }

    if (in >= 0) {
        dup2(in, STDIN_FILENO);
        close(in);
    }

    if (out >= 0) {
        dup2(out, STDOUT_FILENO);
        close(out);
    }

    if (args[0] == NULL)
        exit(1);

    execvp(args[0], args);
    perror("Command execution failed");
    exit(1);
}

int execute(char** args) {

    int pipe_pos = -1;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_pos = i;
            break;
        }
    }

    if (pipe_pos >= 0) {

        args[pipe_pos] = NULL;

        int fd[2];

        if (pipe(fd) == -1) {
            perror("pipe");
            return -1;
        }

        pid_t left = fork();

        if (left == 0) {
            close(fd[0]);
            run_command(args, STDIN_FILENO, fd[1]);
        }

        pid_t right = fork();

        if (right == 0) {
            close(fd[1]);
            run_command(&args[pipe_pos + 1], fd[0], STDOUT_FILENO);
        }

        close(fd[0]);
        close(fd[1]);

        waitpid(left, NULL, 0);
        waitpid(right, NULL, 0);

        return 0;
    }

    pid_t pid = fork();

    if (pid == 0)
        run_command(args, STDIN_FILENO, STDOUT_FILENO);

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    waitpid(pid, NULL, 0);
    return 0;
}
