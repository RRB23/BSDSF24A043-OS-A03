#include "shell.h"

Job jobs[MAX_JOBS];

void add_job(pid_t pid, char* command) {
    for (int i = 0; i < MAX_JOBS; i++) {
        if (!jobs[i].active) {
            jobs[i].pid = pid;
            strncpy(jobs[i].command, command, MAX_LEN - 1);
            jobs[i].command[MAX_LEN - 1] = '\0';
            jobs[i].active = 1;
            printf("[Background] PID: %d\n", pid);
            return;
        }
    }
    printf("Job list is full!\n");
}

void reap_jobs(void) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < MAX_JOBS; i++) {
            if (jobs[i].active && jobs[i].pid == pid) {
                printf("\n[Completed] PID: %d\n", pid);
                jobs[i].active = 0;
                break;
            }
        }
    }
}

void show_jobs(void) {
    reap_jobs();

    printf("\nBackground Jobs:\n");

    for (int i = 0; i < MAX_JOBS; i++) {
        if (jobs[i].active) {
            printf("PID: %d | Command: %s\n",
                   jobs[i].pid, jobs[i].command);
        }
    }
    printf("\n");
}

int handle_builtin(char** args) {

    if (!args || !args[0]) return 1;

    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    if (strcmp(args[0], "cd") == 0) {
        if (!args[1])
            fprintf(stderr, "cd: directory required\n");
        else if (chdir(args[1]) != 0)
            perror("cd");
        return 1;
    }

    if (strcmp(args[0], "help") == 0) {
        printf("Built-ins: cd, exit, help, jobs, history\n");
        return 1;
    }

    if (strcmp(args[0], "jobs") == 0) {
        show_jobs();
        return 1;
    }

    if (strcmp(args[0], "history") == 0) {
        printf("Use Readline history.\n");
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

    for (int i = 0; args[i] != NULL; i++) {

        if (strcmp(args[i], "<") == 0) {
            if (!args[i + 1]) exit(1);

            int fd = open(args[i + 1], O_RDONLY);
            if (fd < 0) {
                perror("input");
                exit(1);
            }

            dup2(fd, STDIN_FILENO);
            close(fd);
            args[i] = NULL;
            break;
        }

        if (strcmp(args[i], ">") == 0) {
            if (!args[i + 1]) exit(1);

            int fd = open(args[i + 1],
                          O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("output");
                exit(1);
            }

            dup2(fd, STDOUT_FILENO);
            close(fd);
            args[i] = NULL;
            break;
        }
    }

    if (!args[0]) exit(1);

    execvp(args[0], args);
    perror("Command execution failed");
    exit(127);
}

int execute(char** args, int background) {

    if (!args || !args[0]) return 0;

    char command[MAX_LEN] = "";

    for (int i = 0; args[i] != NULL; i++) {
        if (strlen(command) + strlen(args[i]) + 2 < MAX_LEN) {
            strcat(command, args[i]);
            strcat(command, " ");
        }
    }

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
            return 1;
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

    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0)
        run_command(args, STDIN_FILENO, STDOUT_FILENO);

    if (background) {
        add_job(pid, command);
        return 0;
    }

    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return 1;
}
